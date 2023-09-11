option = {
  title: {
    text: 'The time of function deploy for Fnproject and BoesFS-FaaS',
    subtext: 'test for 10 times continuously',
    left: 'center'
  },
  tooltip: {
    trigger: 'axis'
  },
  legend: {
    data: ['fn', 'boesfs'],
    top: 70
  },
  grid: {
    left: '3%',
    right: '4%',
    bottom: '3%',
    containLabel: true
  },
  toolbox: {
    feature: {
      saveAsImage: {}
    }
  },
  xAxis: {
    type: 'category',
    boundaryGap: false,
    data: ['1', '2', '3', '4', '5', '6', '7', '8', '9', '10'],
    name: '执行次数',
    nameLocation: 'center'
  },
  yAxis: {
    type: 'value',
    name: '执行时间'
  },
  series: [
    {
      name: 'fn',
      type: 'line',
      data: [0.737, 0.036, 0.032, 0.037, 0.034, 0.036, 0.036, 0.036, 0.042, 0.034],
    },
    {
      name: 'boesfs',
      type: 'line',
      data: [0.096, 0.011, 0.013, 0.011, 0.010, 0.011, 0.010, 0.010, 0.012, 0.012],
    }
  ]
};