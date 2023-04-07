import * as dfd from "danfojs"

import { Flight } from './api'
import { Packet, Char } from './packet'
import * as settings from '../settings'

export class DataStore {
    flight: Flight

    currentTime: Date

    dataframes: {[id: Char]: dfd.DataFrame}

    get startTime(): Date { return this.flight.startTime }
    get launchTime(): Date { return this.flight.launchTime }
    get endTime(): Date | undefined { return this.flight.endTime }

    get currentT(): number { return this.time2T(this.currentTime) }
    set currentT(t: number) {
        this.currentTime = this.t2time(t)
    }
    get startT(): number { return this.time2T(this.startTime) }
    get launchT(): number { return this.time2T(this.launchTime) }
    get endT(): number | undefined {
        return this.endTime && this.time2T(this.endTime)
    }
    get nowT(): number {
        return this.time2T(new Date())
    }

    constructor(flight: Flight) {
        this.flight = flight
        this.currentTime = new Date()
        this.dataframes = {}
    }

    getCurrent(id: Char): any {
        const df = this.dataframes[id]

        // console.log('current', this.currentTime.getTime(), df.values)

        if (!df) return undefined
        const query = df.query(df['time'].lt(this.currentTime.getTime())).tail(1)
        return query.values[0]
    }

    addPackets(packets: { packet: Packet, time: Date, source: string }[]) {
        const packetsById:
            {[id: Char]: { packet: Packet, time: Date, source: string }[]} = {}
        for (const p of packets) {
            if (!p.packet.id) continue
            if (!packetsById[p.packet.id]) packetsById[p.packet.id] = []
            packetsById[p.packet.id].push(p)
        }
        for (const [id, packets] of Object.entries(packetsById)) {
            this.addPacketsById(id, packets)
        }
    }

    addPacketsById(id: Char,
                  packets: { packet: Packet, time: Date, source: string }[]) {

        const format = settings.packetFormats[id]
        const dataframe = this.dataframes[id]
        let columns: string[]

        if (dataframe) {
            columns = dataframe.columns
        }
        else {
            if (format) columns = this.columnsFromFormat(format)
            else columns = this.columnsFromPacket(packets[0].packet)

            console.debug('new dataframe', id, columns)
        }

        const raws = packets.map(p => {
            return this.rawFromPacket(format, columns, p.packet, p.time, p.source)
        })

        if (dataframe) {
            const range = [...Array(raws.length)].map(
                (_, i) => i + dataframe.index.length)
            this.dataframes[id] =
                dataframe.append(raws, range)
        }
        else {
            this.dataframes[id] =
                new dfd.DataFrame(raws, {columns: columns})
        }

    }

    rawFromPacket(format: any, columns: string[],
                  packet: Packet, time: Date, source: String): any[] {
        const raw: any[] = [time.getTime(), source, packet.from, packet.size]

        for (const title of columns.slice(4)) {
            const type = title.slice(0, 1)
            const index = title.length > 1 ? parseInt(title.slice(1)) : 0

            const entry = packet.get(type, index) ?? null
            const entry_format = format?.entries.find((d: any) => d.type == type)
            if (entry_format) {
                if (entry_format.datatype) {
                    const datatype = entry_format.datatype as string
                    raw.push((entry?.payload as any)[datatype])
                }
                else raw.push(true)
            }
            else raw.push(entry?.payload.uint32)
        }
        return raw
    }

    columnsFromFormat(format: any): string[] {
        let columns = ['time', 'source', 'name', 'from', 'size']

        for (const entry of format.entries) {
            if (entry.index)
                columns = columns.concat(entry.index.map(
                    (_: any, i: number) => { return entry.type + i }))
            else
                columns.push(entry.type)
        }
        return columns
    }

    columnsFromPacket(packet: Packet): string[] {
        const columns = ['id', 'time', 'source', 'from', 'size']
        let index = 0;
        let prev_type = ''
        for (const entry of packet.entries.slice(0, -1)) {
            if (entry.type == prev_type) index++
            else index = 0
            prev_type = entry.type
            if (index > 0) columns.push(entry.type + index)
            else columns.push(entry.type)
        }
        return columns
    }


    time2T(time: Date): number {
        return Math.floor(time.getTime() - this.flight.launchTime.getTime())
            / 1000.0
    }

    t2time(time: number): Date {
        return new Date((this.flight.launchTime.getTime() / 1000.0 + time) * 1000.0)
    }
}

class DataSeries {
    private packets: Packet[]

    private cursor: number

    constructor() {
        this.packets = []
        this.cursor = 0
    }
}
