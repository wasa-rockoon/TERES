
export const systemId = "teres1"
export const apiEndpoint = ""
export const wsEndpoint = ""
export const apiEndpointDev = "http://localhost:8888/api"
export const wsEndpointDev = "ws://localhost:8888/api"

export const packetFormats = {
    'A': {
        name: 'Attitude',
        entries: {
            'Q': { name: 'Quaternion',
                   datatype: 'float',
                   indexLabel: ['x', 'y', 'z', 'w']
                 },
            'A': { name: 'Accel',
                   unit: 'm/s^2',
                   datatype: 'float',
                   indexLabel: ['x', 'y', 'z']
                 },
            'g': { name: 'Gyro',
                   unit: 'deg/s',
                   datatype: 'float',
                   indexLabel: ['x', 'y', 'z']
                 },
            'm': { name: 'Mag',
                   unit: 'mT',
                   datatype: 'float',
                   indexLabel: ['x', 'y', 'z']
                 },
        },
    },
}

export const packetList = ['A']

export const charts = []
