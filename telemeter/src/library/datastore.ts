import * as dfd from "danfojs"

import { Flight } from './api'
import { Packet, Char } from './packet'
import * as settings from '../settings'

export class DataStore {
    flight: Flight

    currentTime: Date

    dataseries: { [from: Char]: { [id: Char]: DataSeries } }

    get startTime(): Date { return this.flight.startTime }
    get launchTime(): Date { return this.flight.launchTime }
    get endTime(): Date | undefined { return this.flight.endTime }

    get currentT(): number { return this.time2t(this.currentTime) }
    set currentT(t: number) {
        this.currentTime = this.t2time(t)
    }
    get startT(): number { return this.time2t(this.startTime) }
    get launchT(): number { return this.time2t(this.launchTime) }
    get endT(): number | undefined {
        return this.endTime && this.time2t(this.endTime)
    }
    get nowT(): number {
        return this.time2t(new Date())
    }

    get earliestT(): number | undefined {
        const min = Math.min.apply(
            null,
            this.mapDataseries(d => d.earliest?.t).filter(t => t) as number[]
        )
        if (isFinite(min)) return min
        else return undefined
    }

    get latestT(): number | undefined {
        const max = Math.max.apply(
            null,
            this.mapDataseries(d => d.latest?.t).filter(t => t) as number[]
        )
        if (isFinite(max)) return max
        else return undefined
    }

    mapDataseries<A>(f: (dataseries: DataSeries) => A): A[] {
        return Object.values(this.dataseries).flatMap(ds =>
            Object.values(ds).map(f)
        )
    }

    constructor(flight: Flight) {
        this.flight = flight
        this.currentTime = new Date()
        this.dataseries = {}
    }

    getBy(from: Char, id: Char): DataSeries {
        if (!this.dataseries[from]) this.dataseries[from] = {}
        if (!this.dataseries[from][id])
            this.dataseries[from][id] = new DataSeries(this, from, id)
        return this.dataseries[from][id]
    }

    addPackets(packets: { packet: Packet, time: Date, source: string }[]) {
        const packetsByFromAndId: {[from: Char]: {[id: Char]:
                                                  { packet: Packet,
                                                    time: Date,
                                                    source: string }[]}} = {}
        for (const p of packets) {
            if (!p.packet.from || !p.packet.id) continue
            if (!packetsByFromAndId[p.packet.from])
                packetsByFromAndId[p.packet.from] = {}
            if (!packetsByFromAndId[p.packet.from][p.packet.id])
                packetsByFromAndId[p.packet.from][p.packet.id] = []
            packetsByFromAndId[p.packet.from][p.packet.id].push(p)
        }
        for (const [from, packetsById] of Object.entries(packetsByFromAndId)) {
            if (!this.dataseries[from]) this.dataseries[from] = {}

            for (const [id, packets] of Object.entries(packetsById)) {
                if (!this.dataseries[from][id])
                    this.dataseries[from][id] = new DataSeries(this, from, id)

                this.dataseries[from][id].addPackets(packets.map((p: any) => {
                    p.unixTime = p.time.getTime()
                    return p
                }))
            }
        }
    }

    showT(t: number): string {
        const t_ = Math.abs(t)
        let text = 'T'
        if (t >= 0) text += '+'
        else text += '-'
        const millis = Math.round(t_ * 1000) % 1000
        const seconds = Math.floor(t_) % 60
        const minutes = Math.floor(t_ / 60) % 60
        const hours = Math.floor(t_ / 60 / 60) % 24
        const days = Math.floor(t_ / 60 / 60 / 24)
        if (days > 0) text += days.toString() + ' '
        if (hours > 0) text += hours.toString().padStart(2, '0') + ':'
        text += minutes.toString().padStart(2, '0') + ':'
        text += seconds.toString().padStart(2, '0') + '.'
        text += millis.toString().padStart(3, '0')
        return text
    }


    time2t(time: Date): number {
        return Math.floor(time.getTime() - this.flight.launchTime.getTime())
            / 1000.0
    }

    t2time(t: number): Date {
        return new Date((this.flight.launchTime.getTime() / 1000.0 + t) * 1000.0)
    }
}

interface PacketInfo {
    id: Char
    packet?: Packet
    t?: number
    source?: string
    format: any
    count: number
}

class DataSeries {
    datastore: DataStore
    from: Char
    id: Char
    format: any

    private data: { packet: Packet, unixTime: number, source: string }[]
    private times: number[]
    private values: {[type: Char] : number[][]}

    private sourceAvailables: {[source: string] : boolean}
    private cursor: number

    get sources(): string[] {
        return Object.keys(this.sourceAvailables)
    }

    get earliest(): PacketInfo {
        let p = undefined
        if (this.data.length > 0) p = this.data.at(0)
        return {
            id: this.id,
            packet: p && p.packet,
            t: p && this.datastore.time2t(new Date(p.unixTime)),
            source: p && p.source,
            format: this.format,
            count: p ? 1 : 0
        }
    }

    get latest(): PacketInfo {
        let p = undefined
        if (this.data.length > 0) p = this.data.at(-1)
        return {
            id: this.id,
            packet: p && p.packet,
            t: p && this.datastore.time2t(new Date(p.unixTime)),
            source: p && p.source,
            format: this.format,
            count: this.data.length
        }
    }


    constructor(datastore: DataStore, from: Char, id: Char) {
        this.datastore = datastore
        this.from = from
        this.id = id
        this.format = settings.packetFormats[id]
        this.data = []
        this.sourceAvailables = {}
        this.cursor = 0
        this.times = []
        this.values = {}
        for (const type of Object.keys(this.format?.entries ?? {})) {
            this.values[type] =
                new Array(this.format.entries[type]?.index?.length ?? 1)
                    .fill(0).map(() => [])
        }
    }

    at(time: Date): PacketInfo {
        const index = this.searchIndex(time.getTime())
        let p = undefined
        if (index >= 0) p = this.data[index]
        return {
            id: this.id,
            packet: p && p.packet,
            t: p && this.datastore.time2t(new Date(p.unixTime)),
            source: p && p.source,
            format: this.format,
            count: p ? index + 1 : 0
        }
    }

    getTimes(): number[] {
        return this.times
    }

    getValues(type: number, index: number = 0): number[] {
        return (this.values[type] ?? [])[index]
    }

    // Packets need to be sorted by time
    addPackets(packets: { packet: Packet, unixTime: number, source: string }[]) {
        if (packets.length == 0) return
        this.cursor = this.data.length - 1

        const newValues: {[type: string]: number[][]} = {}

        for (const p of packets) {
            let index = 0
            let prevType = ''
            p.packet.entries.forEach((entry, n) => {
                if (n == p.packet.entries.length - 1 && entry.type == "t") return
                if (entry.type == prevType) index++
                else index = 0
                prevType = entry.type

                const f = this.format?.entries[entry.type]
                if (!f) return

                if (!newValues[entry.type]) {
                    newValues[entry.type] =
                        new Array(f.index?.length ?? 1).fill(0).map(() => [])
                }

                newValues[entry.type][index].push(entry.formatNumber(f))
            })
        }

        for (let i = 0; i < packets.length; i++) {
            const insertAt = this.searchIndex(packets[i].unixTime)
            if (insertAt == this.data.length - 1) {
                this.data.push(...packets.slice(i))
                this.times.push(...packets.slice(i).map(p =>
                    this.datastore.time2t(new Date(p.unixTime))))
                for (const type of Object.keys(this.format?.entries ?? {})) {
                    newValues[type].forEach((values, index) => {
                        this.values[type][index].push(...values.slice(i))
                    })
                }
                break;
            }
            this.data.splice(insertAt, 0, packets[i])
            this.times.splice(insertAt, 0,
                              this.datastore.time2t(new Date(packets[i].unixTime)))
            for (const type of Object.keys(this.format?.entries ?? {})) {
                newValues[type].forEach((values, index) => {
                    this.values[type][index].splice(insertAt, 0, values[i])
                })
            }

            this.cursor = insertAt
        }

        this.cursor = this.data.length - 1
    }

    searchIndex(unixTime: number): number {
        if (this.data.length == 0) return 0
        let step = 0.5
        if (unixTime < this.data[this.cursor].unixTime) {
            while (unixTime < this.data[this.cursor].unixTime) {
                step *= 2
                this.cursor -= step
                if (this.cursor < 0) {
                    this.cursor = 0
                    return -1
                }
            }
            step /= 2
        }
        else {
            while (unixTime > this.data[this.cursor].unixTime) {
                step *= 2
                this.cursor += step
                if (this.cursor >= this.data.length) {
                    this.cursor = this.data.length - 1
                    return this.data.length - 1
                }
            }
            step /= 2
        }

        while (step >= 1) {
            if (unixTime < this.data[this.cursor].unixTime)
                this.cursor -= step
            else if (unixTime > this.data[this.cursor].unixTime)
                this.cursor += step
            else break

            step /= 2
        }
        if (unixTime < this.data[this.cursor].unixTime) this.cursor--;
        return this.cursor
    }
}
