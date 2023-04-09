import * as dfd from "danfojs"

import { Flight } from './api'
import { Packet, Char } from './packet'
import * as settings from '../settings'

export class DataStore {
    flight: Flight

    currentTime: Date

    dataframes: {[id: Char]: dfd.DataFrame}

    dataseries: { [id: Char]: DataSeries }

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
            Object.values(this.dataseries)
                .map(d => d.earliest?.t).filter(t => t) as number[]
        )
        if (isFinite(min)) return min
        else return undefined
    }

    get latestT(): number | undefined {
        const max = Math.max.apply(
            null,
            Object.values(this.dataseries)
                .map(d => d.latest?.t).filter(t => t) as number[]
        )
        if (isFinite(max)) return max
        else return undefined
    }

    constructor(flight: Flight) {
        this.flight = flight
        this.currentTime = new Date()
        this.dataframes = {}
        this.dataseries = {}
    }

    getById(id: Char): DataSeries | undefined {
        return this.dataseries[id]
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
            // this.addPacketsById(id, packets)
            if (!this.dataseries[id])
                this.dataseries[id] = new DataSeries(this, id)

            this.dataseries[id].addPackets(packets.map((p: any) => {
                p.unixTime = p.time.getTime()
                return p
            }))
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
    packet: Packet
    t: number
    source: string
    format: any
}

class DataSeries {
    datastore: DataStore
    id: Char
    format: any

    data: { packet: Packet, unixTime: number, source: string }[]

    private sourceAvailables: {[source: string] : boolean}
    private cursor: number

    get sources(): string[] {
        return Object.keys(this.sourceAvailables)
    }

    get earliest(): PacketInfo | undefined {
        if (this.data.length == 0) return undefined
        const p = this.data[0]
        return p && {
            packet: p.packet,
            t: this.datastore.time2t(new Date(p.unixTime)),
            source: p.source,
            format: this.format
        }
    }

    get latest(): PacketInfo | undefined {
        if (this.data.length == 0) return undefined
        const p = this.data.at(-1)
        return p && {
            packet: p.packet,
            t: this.datastore.time2t(new Date(p.unixTime)),
            source: p.source,
            format: this.format
        }
    }


    constructor(datastore: DataStore, id: Char) {
        this.datastore = datastore
        this.id = id
        this.format = settings.packetFormats[id]
        this.data = []
        this.sourceAvailables = {}
        this.cursor = 0
    }

    at(time: Date): PacketInfo | undefined {
        const index = this.searchIndex(time.getTime())
        if (index < 0) return undefined
        const p = this.data[index]
        return p && {
            packet: p.packet,
            t: this.datastore.time2t(new Date(p.unixTime)),
            source: p.source,
            format: this.format
        }
    }

    // Packets need to be sorted by time
    addPackets(packets: { packet: Packet, unixTime: number, source: string }[]) {
        if (packets.length == 0) return
        this.cursor = this.data.length - 1

        for (let i = 0; i < packets.length; i++) {
            const insertAt = this.searchIndex(packets[i].unixTime)
            if (insertAt == this.data.length - 1) {
                this.data.push(...packets.slice(i))
                break;
            }
            this.data.splice(insertAt, 0, packets[i])
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
