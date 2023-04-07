
export const systemId = "teres1"
export const apiEndpoint = ""
export const wsEndpoint = ""
export const apiEndpointDev = "http://localhost:8888/api"
export const wsEndpointDev = "ws://localhost:8888/api"

export const packetFormats : {[id: string] : any} = {
    'A': {
        name: 'Attitude',
        entries: [
            { type: 'Q',
              name: 'Quaternion',
              datatype: 'float32',
              index: ['x', 'y', 'z', 'w']
            },
            { type: 'A',
              name: 'Accel',
              unit: 'm/s^2',
              datatype: 'float32',
              index: ['x', 'y', 'z']
            },
            { type: 'g',
              name: 'Gyro',
              unit: 'deg/s',
              datatype: 'float32',
              index: ['x', 'y', 'z']
            },
            { type: 'm',
              name: 'Mag',
              unit: 'mT',
              datatype: 'float32',
              index: ['x', 'y', 'z']
            },
        ],
    },
    'l': {
        name: 'Logger',
        entries: [
            { type: 'w', name: 'Wrote', datatype: 'uint' },
            { type: 'd', name: 'Dropped', datatype: 'uint' },
        ]
    },

}

export const packetList = ['A', 'l']

export const charts = []
