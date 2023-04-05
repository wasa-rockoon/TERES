
type Char = string

type Version = 1 | 2 | 3

export class Payload {
    raw: ArrayBuffer
    view: DataView

    constructor() {
        this.raw = new ArrayBuffer(4)
        this.view = new DataView(this.raw)
    }

    get int8(): number {
        return this.view.getInt8(0)
    }
    get int16(): number {
        return this.view.getInt16(0, true)
    }
    get int32(): number {
        return this.view.getInt32(0, true)
    }
    get uint8(): number {
        return this.view.getUint8(0)
    }
    get uint16(): number {
        return this.view.getUint16(0)
    }
    get uint32(): number {
        return this.view.getUint32(0, true)
    }
    get float16(): number {
        const value16: number = this.view.getUint16(0, true)
        let value32: number

        const sign: number = value16 >> 15
        const exp: number = (value16 >> 10) & 0x1F
        const frac: number = value16 & 0x03FF

        if (exp == 0) {
            if (frac == 0) value32 = sign << 31 // +- Zero
            else value32 = (sign << 31) | (frac << 13) // Denormalized values
        }
        else if (exp == 0x1F) {
            if (frac == 0) value32 = (sign << 31) | 0x7F800000 // +- Infinity
            else value32 = (sign << 31) | (0xFF << 23) | (frac << 13) // (S/Q)NaN
        }
        // Normalized values
        else value32 = (sign << 31) | ((exp - 15 + 127) << 23) | (frac << 13)

        const buffer = new ArrayBuffer(4)
        const view = new DataView(buffer)
        view.setUint32(0, value32, true)
        return view.getFloat32(0, true)
    }
    get float32(): number {
        return this.view.getFloat32(0, true)
    }
    set int32(value: number) {
        this.view.setInt32(0, value, true)
    }
    set uint32(value: number) {
        this.view.setUint32(0, value, true)
    }
    set float16(value: number) {
        const buffer = new ArrayBuffer(4)
        const view = new DataView(buffer)
        view.setFloat32(0, value, true)
        const value32: number = view.getUint32(0, true)
        let value16: number

        const sign: number = value32 >> 31
        const exp: number = (value32 >> 23) & 0xFF
        const frac: number = value32 & 0x007FFFFF

        if (exp == 0) {
            if (frac == 0) value16 = sign << 15 // +- Zero
            else value16 = (sign << 15) | (frac >> 13) // Denormalized values
        }
        else if (exp == 0xFF) {
            if (frac == 0) value16 = (sign << 15) | 0x7C0 // +- Infinity
            else value16 = (sign << 15) | (0x1F << 20) | (frac >> 13) // (S/Q)NaN
        }
        // Normalized values
        else value16 = (sign << 15) |
            (Math.max(Math.min(exp - 127 + 15, 0), 31) << 23) | (frac << 13)

        view.setUint16(0, value16, true)
    }
    set float32(value: number) {
        this.view.setFloat32(0, value, true)
    }
}

export class Entry {
    type: Char
    payload: Payload

    constructor() {
        this.type = '@'
        this.payload = new Payload()
    }

    encode(view: DataView): number {
        let len

        const raw = this.payload.view.getUint32(0, true)
        let type = (this.type.charCodeAt(0) - 64) & 0b00111111

        if (raw & 0xFFFF0000) {
            len = 4
            type |= 0b11000000
            view.setUint32(1, raw, true)
        }
        else if (raw & 0xFF00) {
            len = 2
            type |= 0b10000000
            view.setUint16(1, raw, true)
        }
        else if (raw & 0xFF) {
            len = 1
            type |= 0b01000000
            view.setUint8(1, raw)
        }
        else {
            len = 0
        }

        view.setUint8(0, type)

        return len + 1
    }

    decode(view: DataView, version: Version = 3): number {
        let len;
        if (version < 3) {
            this.type = String.fromCharCode(view.getUint8(0) & 0b01111111)
            if (view.getUint8(0) & 0b10000000) len = 0
            else len = 4
        }
        else {
            this.type = String.fromCharCode(view.getUint8(0) & 0b00111111 + 64)
            switch (view.getUint8(0) & 0b11000000) {
                case 0b01000000:
                    len = 1
                    break
                case 0b10000000:
                    len = 2
                    break
                case 0b11000000:
                    len = 4
                    break
                default:
                    len = 0
            }
        }

        switch (len) {
            case 1:
                this.payload.view.setUint8(0, view.getUint8(1))
                break
            case 2:
                this.payload.view.setUint16(0, view.getUint16(1, true), true)
                break
            case 4:
                this.payload.view.setUint32(0, view.getUint32(1, true), true)
                break
        }

        return 1 + len
    }
}

export class Packet {
    id: Char
    from: Char
    size: number
    entries: Entry[]

    constructor(id: Char, from: Char, size: any) {
        this.id = id
        this.from = from
        this.size = size
        this.entries = []
    }

    encode(view: DataView) {
        view.setUint8(0, this.id.charCodeAt(0))
        view.setUint8(1, this.from.charCodeAt(0))
        view.setUint8(2, this.size)

        let i = 3;

        for (const entry of this.entries) {
            const buffer = new ArrayBuffer(5)
            const entry_view = new DataView(buffer)
            const len = entry.encode(entry_view)

            view.setUint8(i, entry_view.getUint8(0))
            view.setUint32(i + 1, entry_view.getUint32(1, true), true)

            i += len
        }
    }

    static decode(view: DataView, version: Version = 3): Packet {
        const id = String.fromCharCode(view.getUint8(0))
        const from = String.fromCharCode(view.getUint8(1))
        const size = view.getUint8(2)
        const packet = new Packet(id, from, size)

        let i = 3;

        for (let n = 0; n < size; n++) {
            const buffer = new ArrayBuffer(5)
            const entry_view = new DataView(buffer)
            entry_view.setUint8(0, view.getUint8(i))
            entry_view.setUint32(1, view.getUint32(i + 1, true), true)
            const entry = new Entry()
            i += entry.decode(entry_view, version)
            packet.entries.push(entry)
        }

        return packet
    }

    static async decodeLogFile(file: File, version: Version = 3)
    : Promise<Packet[]> {
        const buffer = await readFileAsArrayBuffer(file)

        if (version < 2) {
            return []
        }
        else {
            return decodeCOBS(buffer, (buf) => {
                return Packet.decode(new DataView(buf), version)
            })
        }
    }
}


function readFileAsArrayBuffer(file: File): Promise<ArrayBuffer> {
    return new Promise((resolve, reject) => {
        const reader = new FileReader();
        reader.onload = () => {
            resolve(reader.result as ArrayBuffer);
        };
        reader.onerror = reject;
        reader.readAsArrayBuffer(file);
    })
}

function decodeCOBS<A>(buffer: ArrayBuffer, map: (buffer: ArrayBuffer) => A): A[] {
    const result = []

    const len = buffer.byteLength;

    const encoded = new Uint8Array(buffer);

    const decoded_buf = new ArrayBuffer(256);
    const decoded = new Uint8Array(decoded_buf, 0, 256);

    let next_zero = 0xFF;
    let src = 0;
    let rest = 0;
    let dest = 0;

    for (; src < len; src++, rest--) {
        if (rest == 0) {
            const zero = next_zero != 0xFF

            next_zero = encoded[src];
            rest = next_zero;
            if (next_zero == 0) {
                if (dest > 3) result.push(map(decoded_buf))

                dest = 0;
                next_zero = 0xFF;
                rest = 1;
            }
            else if (zero) {
                decoded[dest++] = 0;
            }
        }
        else {
            decoded[dest++] = encoded[src];
        }
    }

    return result
}
