const posList = [
  'left',
  'right',
  'top',
  'bottom',
  'inside',
  'insideTop',
  'insideLeft',
  'insideRight',
  'insideBottom',
  'insideTopLeft',
  'insideTopRight',
  'insideBottomLeft',
  'insideBottomRight'
];

option = {
  title: {
    left: 'center',
    text: 'CPU overhead of Fnproject and BoesFS-FaaS',
    subtext: 'Under different concurrency',
    top: '0'
  },
  tooltip: {
    trigger: 'axis',
    axisPointer: {
      type: 'shadow'
    }
  },
  legend: {
    data: ['fn', 'boesfs'],
    top: 70
  },
  toolbox: {
    show: true,
    orient: 'vertical',
    left: 'right',
    top: 'center',
    feature: {
      mark: { show: true },
      dataView: { show: true, readOnly: false },
      magicType: { show: true, type: ['line', 'bar', 'stack'] },
      restore: { show: true },
      saveAsImage: { show: true }
    }
  },
  xAxis: [
    {
      type: 'category',
      axisTick: { show: false },
      data: ['0', '1', '10', '20', '40', '60', '80', '100'],
      nameGap: '30',
      name: '每秒触发数',
      nameLocation: 'center'
    }
  ],
  yAxis: [
    {
      type: 'value',
      name: 'CPU占用指数'
    }
  ],
  series: [
    {
      name: 'fn',
      type: 'bar',
      barGap: 0,
      emphasis: {
        focus: 'series'
      },
      data: [3.4, 14.7, 70.9, 93.3, 110.9, 120.8, 143.1, 218.3],
    },
    {
      name: 'boesfs',
      type: 'bar',
      emphasis: {
        focus: 'series'
      },
      data: [0.0, 1.3, 9.6, 29.6, 41.5, 72.0, 83.0, 108.6]
    },
  ]
};