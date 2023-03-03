'use strick';

let host = ''
host = 'http://gs.local'

const POLLING_FREQ = 10
const MAX_PLOT_ELEMENTS = 1000
const SERVER_ERROR_MAX = 10
const READ_FILE_BREAK = 1000

// let remote_index = -1;
// let local_index = -1;

let index = -1;

// let remote_packets = {};
// let local_packets = {};

let packets = {};

let timer = null;
let timer2 = null;
let serevr_error_count = 0;

Chart.defaults.global.defaultFontColor = "#fff";

let charts = null;

let mapImage = new Image();
mapImage.src = 'map.png';

const command_format = {
  'B': {
    name: 'Battery',
    entries: {
      'P': { name: 'Vpp', unit: 'V', datatype: 'float' },
      'C': { name: 'Vcc', unit: 'V', datatype: 'float' },
      'D': { name: 'Vdd', unit: 'V', datatype: 'float' },
      'c': { name: 'Vcc', unit: 'A', datatype: 'float' },
      'd': { name: 'Vdd', unit: 'A', datatype: 'float' },
    }
  },
  'P': {
    name: 'GPS',
    entries: {
      'O': { name: 'Longitude',    unit: '°',   datatype: 'int' },
      'A': { name: 'Latitude',     unit: '°',   datatype: 'int' },
      'H': { name: 'Altitude',      unit: 'm',   datatype: 'float' },
      'T': { name: 'Time',         unit: 's',   datatype: 'time' },
      'S': { name: 'Satellites', datatype: 'int' },
    }
  },
  'C': {
    name: 'TWELITE',
    entries: {
      'Q': { name: 'LQI', datatype: 'int' },
      'S': { name: 'Sent',   datatype: 'uint' },
      'R': { name: 'Received',   datatype: 'uint' },
    }
  },
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
    }
  },
  'H': {
    name: 'Air',
    entries: {
      'T': { name: 'Temperature',  unit: '℃',  datatype: 'float' },
      'P': { name: 'Pressure',     unit: 'Pa',  datatype: 'float' },
      'A': { name: 'Presssure Alt', unit: 'm',  datatype: 'float' },
      'h': { name: 'Humidity',     unit: 'RH%', datatype: 'float' },
    }
  },
  'M': {
    name: 'Flight Mode',
    entries: {
      'S': { name: 'Mode', payload: 'Ground' },
      'F': { name: 'Mode', payload: 'Flight' },
    }
  },
  'm': {
    name: 'Mode',
    entries: {
      'N': { name: 'Launch', payload: 'No', warning: true },
      'A': { name: 'Launch', payload: 'Allow' },
      'S': { name: 'Mode', payload: 'Ground', warning: true },
      'F': { name: 'Mode', payload: 'Flight' },
    }
  },
  'l': {
    name: 'Logger',
    entries: {
      'w': { name: 'Wrote', datatype: 'uint' },
      'd': { name: 'Dropoed', datatype: 'uint' },
    }
  },


  // 'Sample': {
  //   name: 'Sample',
  //   entries: {
  //     'P': { name: 'Vpp', unit: 'V', datatype: 'float', normal: 9.0 },
  //     'C': { name: 'Vcc', unit: 'V', datatype: 'float', normal: 5.0 },
  //     'D': { name: 'Vdd', unit: 'V', datatype: 'float', normal: 3.3 },
  //     'c': { name: 'Vcc', unit: 'A', datatype: 'float', },
  //     'd': { name: 'Vdd', unit: 'A', datatype: 'float', },
  //   }
  // },
}

function chartConfig() {
  let c = [
    {
      name: 'Altitude',
      x: 't',
      y: ['H.A', 'P.H'],
      xLabel: 't [s]',
      yLabel: 'Altitude [m]'
    },
    {
      name: 'Accelaration',
      x: 't',
      y: ['A.A.0', 'A.A.1', 'A.A.2'],
      xLabel: 't [s]',
      yLabel: 'Accelaration [m/s^2]'
    },
    {
      name: 'Location',
      x: 'P.O',
      y: ['P.A'],
      xLabel: 'Longitude [deg]',
      yLabel: 'Latitude [deg]',
      // xMin: 132,
      // xMax: 134,
      // yMin: 32,
      // yMax: 34,
      aspect: 1,
      // background: mapImage,
    },
    {
      name: 'Pressure',
      x: 't',
      y: ['H.P'],
      xLabel: 't [s]',
      yLabel: 'Pressure [Pa]'
    },
    {
      name: 'Temperature',
      x: 't',
      y: ['H.T'],
      xLabel: 't [s]',
      yLabel: 'Temperature [℃]'
    },
    {
      name: 'Satelites',
      x: 't',
      y: ['P.S'],
      xLabel: 't [s]',
      yLabel: 'Satelites'
    },
  ];
  return c;
}

function createCharts() {
  charts = chartConfig();

  $('#charts').empty();

  for (const chart of charts) {
    const canvas = $('<canvas></canvas>');
    $('#charts').append(canvas);

    function drawBackground(target) {
      if (chart.background) {
        // console.log(target)
        target.ctx.drawImage(
          chart.background,
          target.chartArea.left,
          target.chartArea.top,
          target.chartArea.right - target.chartArea.left,
          target.chartArea.bottom - target.chartArea.top,
        );
      }
    }

    const data = {
      lebals: [],
      datasets: chart.y.map(y => {
        const ids = y.split('.');
        const command = command_format[ids[0]];
        if (!command) console.error(ids[0], 'not found');
        return {
          id: ids[0],
          entry_type: ids[1],
          index: ids.length > 2 ? ids[2] : null,
          label: command.entries[ids[1]].name +
            (ids.length > 2 ? '.' + ids[2] : ''),
          data: [],
          reducing: [],
          reduce_rate: 1,
          showLine: true,
          fill: false,
          pointRadius: 0,
          lineTension: 0,
        }
      })
    }
    const config = {
      type: chart.type || 'scatter',
      data: data,
      options: {
        title: {
          display: true,
          text: chart.name,
        },
        element: {
          point: {
            radius: 0,
          }
        },
        legend: {
          display: chart.y.length > 1
        },
        scales: {
          xAxes: [{
            display: true,
            gridLines: {
              color: '#606060'
            },
            ticks:  {
              min: chart.xMin,
              max: chart.xMax
            },
            scaleLabel: {
              display: true,
              labelString: chart.xLabel
            }
          }],
          yAxes: [{
            display: true,
            gridLines: {
              color: '#606060'
            },
            ticks:  {
              min: chart.yMin,
              max: chart.yMax
            },
            scaleLabel: {
              display: true,
              labelString: chart.yLabel
            }}
          ]
        },
        animation: false,
      },
      plugins: [{
        beforeDraw: drawBackground
      }],
    }

    chart.chart = new Chart(canvas, config);
  }
}

function addHistory(command) {

  for (chart of charts) {
    if (true) {
      // console.log(chart, command)
      const x_entry = command.entries.find(entry => entry.type == chart.x)
      if (!x_entry) {
        // console.log('Can\'t find x', chart.id, chart.x);
        continue;
      }
      const x = x_entry.payload;
      chart.chart.data.datasets.forEach((dataset) => {
        if (dataset.id != command.id) return;

        let y_entry = command.entries.find(
          entry => entry.type == dataset.entry_type);

        if (dataset.index) {
          y_entry = command.entries.find(
            entry => entry.type == dataset.entry_type
              && entry.index == dataset.index );
        }

        // console.log(command.entries, dataset.entry_type)
        if (!y_entry) {
          console.log('Can\'t find y', chart.y, dataset.entry_type);
          return;
        }
        const y = y_entry.payload;

        if (chart.name == 'Pressure' && y < 100) {
          console.log(chart.name, dataset, command.entries, y_entry.type, x, y, )
        }

        if (Math.abs(x) > 1E8 || Math.abs(y) > 1E8) return;
        if (chart.xDataMin != undefined && x < chart.xDataMin) return;
        if (chart.xDataMax != undefined && x > chart.xDataMax) return;
        if (chart.yDataMin != undefined && y < chart.yDataMin) return;
        if (chart.yDataMax != undefined && y > chart.yDataMax) return;

        if (dataset.reducing.length == dataset.reduce_rate) {
          let x_reduced = 0;
          let y_reduced = 0;
          for (let d of dataset.reducing) {
            x_reduced += d.x;
            y_reduced += d.y;
          }
          dataset.reducing = [];
          dataset.data.push({x: x_reduced / dataset.reduce_rate,
                             y: y_reduced / dataset.reduce_rate});
        }
        else {
          dataset.reducing.push({x, y});
        }

        if (dataset.data.length > MAX_PLOT_ELEMENTS) {
          let data = [];

          dataset.reduce_rate *= 2;
          console.log("reduce rate: ", dataset.reduce_rate);

          for (let i = 0; i < Math.floor(dataset.data.length / 2); i++) {
            if (i * 2 + 1 == dataset.data.length) {
              data.push(dataset.data[i * 2]);
              break;
            }
            else {
              data.push({
                x: (dataset.data[i * 2].x + dataset.data[i * 2 + 1].x) / 2,
                y: (dataset.data[i * 2].y + dataset.data[i * 2 + 1].y) / 2,
              })
            }
          }
          console.log('reduced', data.length, dataset.reduce_rate);
          dataset.data = data;
        }
      })
    }
  }
}

function main() {
  fetchStart();

  $('#connection-error').hide();
  $('#retry-button').on('click', fetchStart);
  $('#select-file').on('change', readFile);

  // let sample_format = command_format['Sample'];
  // let sample = { name: sample_format.name, id: 'S', to: 'G', from: 'G',
  //                size: 5, entries: [], t: 1234567};
  // for (let k in sample_format.entries) {
  //   let entry = {};
  //   Object.assign(entry, sample_format.entries[k]);
  //   entry.payload = Math.random();
  //   sample.entries.push(entry)
  // }
  // console.log(sample);

  createCharts();
  render();
  updateCharts();
}

function readFile() {
  fetchCancel();

  createCharts();

  var file = $('#select-file')[0].files[0];
  if(!file) {
    alert('No file');
    return;
  }
  var reader = new FileReader();
  reader.readAsArrayBuffer(file);
  reader.onload = () => {

    const len = reader.result.byteLength;
    // const len = 10000;

    var encoded = new Uint8Array(reader.result, 0, len);

    var decoded_buf = new ArrayBuffer(256);
    var decoded = new Uint8Array(decoded_buf, 0, 256);

    var next_zero = 0xFF;
    var src = 0;
    var rest = 0;
    var dest = 0;

    console.log(len);
    // console.log(encoded);

    var packet_bufs = [];


    num = 0;

    for (; src < len; src++, rest--) {
      if (rest == 0) {
        if (next_zero != 0xFF) {
          next_zero = encoded[src];
          rest = next_zero;
          if (next_zero == 0) {
            addCommand(decoded.slice(1, dest));
            num++;
            dest = 0;
            next_zero = 0xFF;
            rest = 1;
          }
          else {
            decoded[dest++] = 0;
          }
        }
        else {
          next_zero = encoded[src];
          rest = next_zero;
          if (next_zero == 0) {
            addCommand(decoded.slice(1, dest));
            num++;
            dest = 0;
            next_zero = 0xFF;
            rest = 1;
          }
          else {
            decoded[dest++] = 0;
          }
        }
      }
      else {
        decoded[dest++] = encoded[src];
      }
    }

    // console.log(packet_bufs);

    console.log('returned', num);

    $('#connection-error').hide();
    serevr_error_count = 0;

    render();
    updateCharts();

    // let lines = reader.result.split('\n').filter(line => line != "");
    // console.log('File read', lines.length);
    // createCharts();

    // const size = lines.length;

    // function add() {
    //   console.log(`${size - lines.length} / ${size}`);
    //   for (let n = 0; n < READ_FILE_BREAK; n++) {
    //     if (lines.length == 0) {
    //       console.log("complete");
    //       return;
    //     }
    //     let line = lines.shift();
    //     let command = addHexCommand(line);
    //   }
    //   render();
    //   updateCharts();
    //   setTimeout(add, 1);
    //   // if (command.id == 'n') console.log(command);
    // }

    // add();

    // // addHexCommands('0\n' + reader.result, false);

    // setTimeout(() => {
    //   fetchCancel();
    //   $('#connection-error').hide();

    //   render();
    //   updateCharts();
    // }, 1000)

  }
}

function fetchStart() {
  $('#retry-button').hide();
  serevr_error_count = 0;

  fetchCancel();

  fetch();
}

function fetchCancel() {
  if (timer) clearInterval(timer);
  if (timer2) clearTimeout(timer2);
}

function fetch() {
  // local_packets['Sample'].updated = true;

    for (let id in packets) {
      packets[id].updated = false;
    }
  // }

  // if (local) console.log('fetch local',  local_index);
  // else console.log('fetch remote',  remote_index);

  console.log('fetch', index);

  $.ajax({
    url: host + '/messages',
    type: 'GET',
     data: { index: index },
    // data: { index: local ? local_index : remote_index },
    dataType: 'text',
    async: true,
    cache: false,
    timeout: 1000 / POLLING_FREQ})
    .then(str => {
      addHexCommands(str);
      render();
      updateCharts();

      timer = setTimeout(() => {
        fetch();
      }, 1000 / POLLING_FREQ);
    })
    .catch(e => {
      $('#connection-error').show();
      serevr_error_count++;

      timer = setTimeout(() => {
        fetch();
      }, 1000 / POLLING_FREQ);

      if (serevr_error_count > SERVER_ERROR_MAX) {
        fetchCancel();
        $('#retry-button').show();
      }

      console.error(e.statusText);
      render();
    });
}

function addHexCommands(str) {
  $('#connection-error').hide();
  serevr_error_count = 0;

  const lines = str.split('\n').filter(line => line != "");

  console.log('returned', parseInt(lines[0]), lines.length);

  // if (local) local_index = parseInt(lines[0]);
  // else remote_index = parseInt(lines[0]);

  index = parseInt(lines[0]);

  lines.shift();

  lines.forEach((line, _) => {
    let command = addHexCommand(line);
    if (lines.length < 50) {
        // console.log(command);
    }
  });
}

function addCommand(buf) {
  let command = parseCommand(buf);

  // console.log(buf, command);

  if (!command) return null;

  command.updated = true;
  command.local = command.from == '\0';

  addHistory(command);

  // if (!command.local) {
  packets[command.id] = command;
  // }

  return command;
}


function addHexCommand(line) {
  let command = parseCommandHex(line);

  if (!command) return null;

  command.updated = true;
  command.local = command.from == '\0';

  addHistory(command);

  if (!command.local) {
    packets[command.id] = command;
  }

  return command;
}

function updateCharts() {
  for (chart of charts) {
    chart.chart.update();
    if (chart.aspect) {
      chart.chart.aspectRatio = chart.aspect;
      chart.chart.resize();
    }
  }
}

function render() {
  // $('#rocket-packets').empty();
  // for (let id in rocket_packets) {
  //   $('#rocket-packets').append(commandItem(rocket_packets[id]));
  // }
  $('#packets').empty();
  for (let id in packets) {
    commandItem(packets[id]).appendTo('#packets');
  }
}

function commandItem(command) {
  let entries = [];
  let diagnostics = [];

  for (const entry of command.entries) {
    if (entry.datatype == 'diag') diagnostics.push(entry)
    else entries.push(entry);
  }

  const dom =
        $(`<table class="pure-table pure-table-horizontal command
                         ${command.updated ? 'updated' : ''}">
            <thead>
               <tr>
                 <th colspan="3">${command.name}</th>
                 <th colspan="2">${showTime(command.t)}</th>
               </tr>
            </thead>
            ${entries.map(entryItem).join('\n')}
            ${diagnostics.map(diagnosticsItem).join('\n')}
          </table>`);
  dom.on('click', () => { console.log(command); });
  return dom;
}
function entryItem(entry) {
  if (entry.type == 't') return '';

  if (entry.datatype == 'bytes') {
    let dom = '';
    entry.payload.forEach((payload, i) => {
      let format = entry.bytes[i];

      let class_ = '';
      if (format.error && format.error[payload])
        class_ += 'error ';
      if (format.warning && format.warning[payload])
        class_ += 'warning ';

      if (format.enum) {
        payload = format.enum[payload];
      }

      dom +=
        `<tr class="${class_}">
           <td colspan="2">${format.name}</td>
           <td>${format.unit ? '[' + format.unit + ']' : ''}</td>
           <td colspan="2">${payload == null ? '' : payload}</td>
         </tr>`;
    })
    return dom;
  }
  else {
    let payload = entry.payload;
    switch (entry.datatype) {
    case 'float':
      payload = String(entry.payload).slice(0, 9);
      break;
    case 'time':
      payload = payload.toLocaleTimeString('ja-JP');
    default:
      break;
    }

    let class_  = '';
    if (entry.error) class_ += 'error ';
    if (entry.warning) class_ += 'warning';

    let index = '';
    if (Array.isArray(entry.indexLabel))
      index = entry.indexLabel[entry.index];
    else if (entry.indexLabel)
      index = String(entry.index);

    return `<tr class="${class_}">
            <td colspan="2">${entry.name} ${index}</td>
            <td>${entry.unit ? '[' + entry.unit + ']' : ''}</td>
            <td colspan="2">${payload == null ? '' : payload}</td>
          </tr>`;
  }

}
function diagnosticsItem(entry) {
  if (entry.type == 't') return '';

  let dom = '';

  function light(name, error) {
    if (name == null) name = '';
    return `<td class="${error ? 'error' : ''}">${name}</td>`;
  }

  // console.log(entry);

  diagnostics.forEach((format, n) => {
    let module = entry.modules[n];
    dom += `<tr class="diag"> ${light(format.name, module.error)}`;
    format.status.forEach((f, i) => {
      dom += light(f, module.status[i] || module.error);
    })
    dom += `</tr>`;
  })

  return dom;
}


function parseCommand(bytes) {

  const id = String.fromCharCode(bytes[0]);
  const from = String.fromCharCode(bytes[1]);
  const size = bytes[2];

  const format = command_format[id];
  let command = {id: id, to: null, from: from, size: size,
                 name: format && format.name,
                 hex: bytes, t: null,
                 entries: []}

  let indices = {};

  let i = 3;
  while (true) {
    if (i >= bytes.length) break;

    const type = String.fromCharCode(bytes[i] & 0b01111111);

    const index = indices[type] == undefined ? 0 : indices[type] + 1;
    indices[type] = index;

    let entry = {type, index};

    if (format && format.entries[type]) {
      Object.assign(entry, format.entries[type]);
    }
    else {
      entry.name = type;
    }

    if ('0' <= type && type <= '9') entry.datatype = 'diag';


    let buf = new ArrayBuffer(4);
    let view = new DataView(buf);

    if (bytes[i] & 0b10000000) {
      if (entry.payload == undefined) {
        view.setUint32(0, 0);
      }

      i += 1;
    }
    else {
      for (let n = 0; n < 4; n++) {
        view.setUint8(3 - n, bytes[i + n + 1]);
      }

      i += 5;
    }

    if (entry.payload != undefined) {
    }
    else if (type == 't') {
      entry.payload = view.getUint32(0) / 1000.0;
      command.t = entry.payload;
    }
    else if (entry.datatype) {
      switch (entry.datatype) {
      case 'float':
        entry.payload = view.getFloat32(0);
        break;
      case 'int':
        entry.payload = view.getInt32(0);
        break;
      case 'uint':
        entry.payload = view.getUint32(0);
        break;
      case 'time':
        entry.payload = new Date(view.getUint32(0) * 1000);
        break;
      case 'bytes':
        entry.payload = [0, 1, 2, 3].map(i => view.getUint8(3 - i));
        break;
      case 'diag':
        entry.payload = view.getUint32(0);
        const modules = view.getUint8(0);

        const status_bits = [];
        for (let n = 0; n < 3; n++) {
          status_bits.push((view.getUint8(3 - n) & 0b1111));
          status_bits.push((view.getUint8(3 - n) >>> 4) & 0b1111);
        }

        entry.modules = [];
        for (let n = 0; n < diagnostics.length; n++) {
          let s = [];
          for (let i = 0; i < 4; i++) {
            s.push(((status_bits[n] & (1 << i)) != 0));
          }
          entry.modules.push({
            error: (modules & (1 << n)) != 0,
            status: s,
          })
          // console.log(modules.toString(2), entry, entry.payload.toString(2), status_bits);
          // fetchCancel();
        }
        break;
      default:
        // entry.payload = line.slice(i * 2 + 2, i * 2 + 12);
        entry.payload = [0, 1, 2, 3].map(i => view.getUint8(3 - i));
        break;
      }
    }
    else {
      // entry.payload = line.slice(i * 2 + 2, i * 2 + 12);
      entry.payload = [0, 1, 2, 3].map(i => view.getUint8(3 - i));
    }

    command.entries.push(entry);
  }

  // console.log(command)

  return command;
}



function parseCommandHex(line) {
  const hexes = line.replace(/\r?\n/g, '').match(/.{2}/g);
  if (!hexes) return null;
  const bytes = hexes.map(hex => parseInt(hex, 16));

  const id = String.fromCharCode(bytes[1]);
  const to = String.fromCharCode(bytes[2]);
  const from = String.fromCharCode(bytes[3]);
  const size = bytes[4];

  const format = command_format[id];
  let command = {id: id, to: to, from: from, size: size,
                 name: format && format.name,
                 hex: line, t: null,
                 entries: []}

  let indices = {};

  let i = 5;
  while (true) {
    if (i >= bytes.length) break;

    const type = String.fromCharCode(bytes[i] & 0b01111111);

    const index = indices[type] == undefined ? 0 : indices[type] + 1;
    indices[type] = index;

    let entry = {type, index};

    if (format && format.entries[type]) {
      Object.assign(entry, format.entries[type]);
    }
    else {
      entry.name = type;
    }

    if ('0' <= type && type <= '9') entry.datatype = 'diag';


    let buf = new ArrayBuffer(4);
    let view = new DataView(buf);

    if (bytes[i] & 0b10000000) {
      if (entry.payload == undefined) {
        view.setUint32(0, 0);
      }

      i += 1;
    }
    else {
      for (let n = 0; n < 4; n++) {
        view.setUint8(3 - n, bytes[i + n + 1]);
      }

      i += 5;
    }

    if (entry.payload != undefined) {
    }
    else if (type == 't') {
      entry.payload = view.getUint32(0) / 1000.0;
      command.t = entry.payload;
    }
    else if (entry.datatype) {
      switch (entry.datatype) {
      case 'float':
        entry.payload = view.getFloat32(0);
        break;
      case 'int':
        entry.payload = view.getInt32(0);
        break;
      case 'uint':
        entry.payload = view.getUint32(0);
        break;
      case 'time':
        entry.payload = new Date(view.getUint32(0) * 1000);
        break;
      case 'bytes':
        entry.payload = [0, 1, 2, 3].map(i => view.getUint8(3 - i));
        break;
      case 'diag':
        entry.payload = view.getUint32(0);
        const modules = view.getUint8(0);

        const status_bits = [];
        for (let n = 0; n < 3; n++) {
          status_bits.push((view.getUint8(3 - n) & 0b1111));
          status_bits.push((view.getUint8(3 - n) >>> 4) & 0b1111);
        }

        entry.modules = [];
        for (let n = 0; n < diagnostics.length; n++) {
          let s = [];
          for (let i = 0; i < 4; i++) {
            s.push(((status_bits[n] & (1 << i)) != 0));
          }
          entry.modules.push({
            error: (modules & (1 << n)) != 0,
            status: s,
          })
          // console.log(modules.toString(2), entry, entry.payload.toString(2), status_bits);
          // fetchCancel();
        }
        break;
      default:
        entry.payload = line.slice(i * 2 + 2, i * 2 + 12);
        break;
      }
    }
    else {
      entry.payload = line.slice(i * 2 + 2, i * 2 + 12);
    }

    command.entries.push(entry);
  }

  // console.log(command)

  return command;
}

function showTime(t) {
  const ms = Math.floor(t * 100) % 100;
  const s = Math.floor(t) % 60;
  const m = Math.floor(t / 60) % 60;
  const h = Math.floor(t / 60 / 60) % 24;

  return String(h) + ':'
    + String(m).padStart(2, '0') + ':'
    + String(s).padStart(2, '0') + '.'
    + String(ms).padStart(2, '0');
}

window.addEventListener('load', main);

