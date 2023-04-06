import axios from 'axios';
import * as settings from '@/settings'
import { Packet } from '@/library/packet'

const API_URL =
    process.env.NODE_ENV == 'development'
    ? settings.apiEndpointDev : settings.apiEndpoint
const WS_URL =
    process.env.NODE_ENV == 'development'
    ? settings.wsEndpointDev : settings.wsEndpoint

axios.defaults.baseURL = API_URL;
axios.defaults.headers.put['Content-Type'] = 'application/json;charset=utf-8';
axios.defaults.headers.put['Access-Control-Allow-Origin'] = '*';


interface Flight {
    id: string,
    name: string,
    system: System | string,
    startTime: Date,
    launchTime: Date,
    endTime?: Date,
}

interface System {
    id: string,
    name: string,
    activeFlight?: Flight,
    client: string,
}

interface NewSystem {
    id: string,
    name: string,
    password: string,
    client: string,
}

class API {
    async getSystems(): Promise<System[]> {
        const response = await axios.get<{ systems: System[] }>(`/systems/`)
        return response.data.systems
    }

    async getSystem(system_id: string): Promise<System> {
        const response = await axios.get<System>(
            `/systems/${system_id}`,
            { params: { password: localStorage.password} })
        return response.data
    }

    async putSystem(system: NewSystem): Promise<System> {
        const response = await axios.put<System>(
            `/systems/${system.id}`, null, { params: system })
        return response.data
    }

    async getFlights(system_id: string): Promise<Flight[]> {
        const response = await axios.get<{flights: any[]}>(
            `/systems/${system_id}/flights/`,
            { params: { password: localStorage.password} })
        return response.data.flights.map(this.makeFlight)
    }


    async getFlight(id: string): Promise<Flight> {
        const response = await axios.get<Flight>(
            `/flights/${id}`,
            { params: { password: localStorage.password} })
        return this.makeFlight(response.data)
    }

    async postFlight(system_id: string, name: string, activate: boolean = false)
    : Promise<Flight> {
        const response = await axios.post<Flight>(
            `/systems/${system_id}/flights/`, null,
            { params: {
                name: name,
                activate: activate,
                password: localStorage.password,
            }
            })
        return this.makeFlight(response.data)
    }

    async connect(flightId: string,
                  source: string,
                  startTime?: Date,
                  endTime?: Date,
                  callback?: (packet: Packet, time: Date) => void)
    : Promise<Connection> {
        return new Connection(flightId, source, startTime, endTime, callback)
            .open()
    }



    makeFlight(flight: any): Flight {
        flight.startTime = new Date(flight.startTime)
        flight.launchTime = new Date(flight.launchTime)
        if (flight.endTime) flight.endTime = new Date(flight.endTime)
        return flight
    }
}


type PacketCallback = (packet: Packet, time: Date, source: string) => void

class Connection {
    private socket?: WebSocket
    private flightId: string
    private source: string
    private startTime?: Date
    private endTime?: Date
    private callback: PacketCallback

    constructor(flightId: string,
                source: string,
                startTime?: Date,
                endTime?: Date,
                callback?: PacketCallback) {
        this.flightId = flightId
        this.source = source
        this.startTime = startTime
        this.endTime = endTime
        this.callback = callback ?? (() => {})
    }

    async open(): Promise<Connection> {
        return new Promise((resolve, reject) => {
            const params = new URLSearchParams({
                source: this.source,
                password: localStorage.password,
            })
            if (this.startTime)
                params.append('startTime', this.startTime.toISOString())
            if (this.endTime)
                params.append('endTime', this.endTime.toISOString())

            this.socket = new WebSocket(
                `${WS_URL}/flights/${this.flightId}/packets?${params.toString()}`)
            this.socket.binaryType = "arraybuffer";

            this.socket.onopen = e => {
                resolve(this)
            }
            this.socket.onerror = e => {
                console.error(e)
                // this.open()
            }
            this.socket.onmessage = e => {

                if (e.data instanceof ArrayBuffer) {
                    const buffer = new ArrayBuffer(256)
                    new Uint8Array(buffer).set(new Uint8Array(e.data))
                    const unixTime =
                        Number(new DataView(buffer).getBigInt64(0, true))
                    const time = new Date(unixTime)
                    const source = String.fromCharCode.apply(
                        null, Array.from(new Uint8Array(buffer, 8, 8)))
                    const packet = Packet.decode(new DataView(buffer, 16))
                    this.callback(packet, time, source)
                }
            }
            this.socket.onclose = e => {
                console.log('closed', e)
            }
        })
    }

    send(packet: Packet, time: Date) {
        const unixTime = time.getTime()
        const buffer = new ArrayBuffer(256)
        new DataView(buffer).setBigInt64(0, BigInt(unixTime), true)
        const len = packet.encode(new DataView(buffer, 8))
        this.socket?.send(new DataView(buffer, 0, 8 + len))
    }

    close() {
        this.socket?.close()
    }
}


export const api = new API()
