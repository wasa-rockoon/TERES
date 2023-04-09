
export const systemId = "teres1"
export const apiEndpoint = ""
export const wsEndpoint = ""
export const apiEndpointDev = "http://localhost:8888/api"
export const wsEndpointDev = "ws://localhost:8888/api"

export const packetFormats : {[id: string] : any} = {
    'A': {
        name: 'Attitude',
        entries: {
            'Q': {
                name: 'Quaternion',
                datatype: 'float32',
                index: ['x', 'y', 'z', 'w']
            },
            'A': {
              name: 'Accel',
              unit: 'm/s^2',
              datatype: 'float32',
              index: ['x', 'y', 'z']
            },
            'g': {
              name: 'Gyro',
              unit: 'deg/s',
              datatype: 'float32',
              index: ['x', 'y', 'z']
            },
            'm': {
              name: 'Mag',
              unit: 'mT',
              datatype: 'float32',
              index: ['x', 'y', 'z']
            },
        },
    },
    'P': {
        name: 'GPS',
        entries: {
            'O': { name: 'Longitude',    unit: '°',   datatype: 'int32' },
            'A': { name: 'Latitude',     unit: '°',   datatype: 'int32' },
            'H': { name: 'Altitude',      unit: 'm',   datatype: 'float32' },
            'T': { name: 'Time',         unit: 's',   datatype: 'uint32' },
            'S': { name: 'Satellites', datatype: 'int32' },
        }
    },
    'C': {
        name: 'TWELITE',
        entries: {
            'Q': { name: 'LQI', datatype: 'uint8' },
            'S': { name: 'Sent',   datatype: 'uint32' },
            'R': { name: 'Received',   datatype: 'uint32' },
        }
    },
    'H': {
        name: 'Air',
        entries: {
            'T': { name: 'Temperature',  unit: '℃',  datatype: 'float32' },
            'P': { name: 'Pressure',     unit: 'Pa',  datatype: 'float32' },
            'A': { name: 'Presssure Alt', unit: 'm',  datatype: 'float32' },
            'h': { name: 'Humidity',     unit: 'RH%', datatype: 'float32' },
        }
    },
    'l': {
        name: 'Logger',
        entries: {
            't': { name: 'Time', unit: 's', datatype: 'uint32' },
            'w': { name: 'Wrote', datatype: 'uint32' },
            'd': { name: 'Dropped', datatype: 'uint32' },
        }
    },

}

export const packetList = ['A', 'P', 'H', 'C', 'l']

export const charts = []
