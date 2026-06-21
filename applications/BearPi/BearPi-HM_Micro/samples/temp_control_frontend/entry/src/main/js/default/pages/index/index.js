import app from '@system.app';

const PARAMS = [
  { label: '箱体外环 Kp', command: 'tune box kp ', key: 'box_kp' },
  { label: '箱体外环 Ki', command: 'tune box ki ', key: 'box_ki' },
  { label: '箱体外环 Kd', command: 'tune box kd ', key: 'box_kd' },
  { label: 'PTC 内环 Kp', command: 'tune heat kp ', key: 'heat_kp' },
  { label: 'PTC 内环 Ki', command: 'tune heat ki ', key: 'heat_ki' },
  { label: 'PTC 内环 Kd', command: 'tune heat kd ', key: 'heat_kd' },
  { label: '制冷风扇 Kp', command: 'tune cool kp ', key: 'cool_kp' },
  { label: '制冷风扇 Ki', command: 'tune cool ki ', key: 'cool_ki' },
  { label: '回差温度', command: 'tune hys ', key: 'hysteresis_band' },
  { label: '保温偏置', command: 'tune warmbias ', key: 'warming_bias' },
  { label: '升温偏置', command: 'tune heatbias ', key: 'heating_bias' }
];

export default {
  data: {
    currentTemperature: '--',
    targetTemperature: '40.0',
    pendingTarget: 40,
    ptcTemperature: '--',
    ptcTargetTemperature: '--',
    currentPwm: '--',
    controlState: '--',
    ptcState: '--',
    hysteresisBand: '--',
    connectionText: '未连接',
    updatedAt: '等待数据',
    message: '',
    paramIndex: 0,
    selectedParamLabel: PARAMS[0].label,
    selectedParamValue: '--',
    status: null,
    timer: null,
    busy: false
  },

  onInit() {
    this.refreshStatus();
    this.timer = setInterval(() => {
      this.refreshStatus();
    }, 1000);
  },

  onDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
      this.timer = null;
    }
  },

  sendCommand(command, onSuccess) {
    if (this.busy && command === 'get_status') {
      return;
    }
    this.busy = true;
    app.tempcontrol({
      command: command,
      success: (res) => {
        this.busy = false;
        this.connectionText = '已连接';
        this.message = '';
        if (onSuccess) {
          onSuccess(res.response);
        }
      },
      fail: (err) => {
        this.busy = false;
        this.connectionText = '连接失败';
        this.message = err && err.message ? err.message : '命令失败';
      },
      complete: () => {}
    });
  },

  refreshStatus() {
    this.sendCommand('get_status', (response) => {
      let next = null;
      try {
        next = JSON.parse(response);
      } catch (err) {
        this.message = '状态数据解析失败';
        return;
      }
      this.status = next;
      this.currentTemperature = this.format(next.current_temperature, 1);
      this.targetTemperature = this.format(next.target_temperature, 1);
      this.pendingTarget = Number(next.target_temperature);
      this.ptcTemperature = this.format(next.current_ptc_temperature, 1);
      this.ptcTargetTemperature = this.format(next.ptc_target_temperature, 1);
      this.currentPwm = this.format(Number(next.current_pwm) * 100, 0);
      this.controlState = next.control_state || '--';
      this.ptcState = next.ptc_state || '--';
      this.hysteresisBand = this.format(next.hysteresis_band, 1);
      this.updatedAt = '刚刚更新';
      this.syncSelectedParam();
    });
  },

  targetDown() {
    this.pendingTarget = this.clamp(Number(this.pendingTarget) - 1, 20, 90);
    this.targetTemperature = this.format(this.pendingTarget, 1);
  },

  targetUp() {
    this.pendingTarget = this.clamp(Number(this.pendingTarget) + 1, 20, 90);
    this.targetTemperature = this.format(this.pendingTarget, 1);
  },

  applyTarget() {
    const value = this.format(this.pendingTarget, 1);
    this.sendCommand('tune target ' + value, () => {
      this.message = '目标温度已设定';
      this.refreshStatus();
    });
  },

  selectPrevParam() {
    this.paramIndex = (this.paramIndex + PARAMS.length - 1) % PARAMS.length;
    this.syncSelectedParam();
  },

  selectNextParam() {
    this.paramIndex = (this.paramIndex + 1) % PARAMS.length;
    this.syncSelectedParam();
  },

  paramDownLarge() {
    this.adjustParam(-0.1);
  },

  paramDown() {
    this.adjustParam(-0.01);
  },

  paramUp() {
    this.adjustParam(0.01);
  },

  paramUpLarge() {
    this.adjustParam(0.1);
  },

  adjustParam(delta) {
    let value = Number(this.selectedParamValue);
    if (isNaN(value)) {
      value = 0;
    }
    value = this.clamp(value + delta, 0, 120);
    this.selectedParamValue = this.format(value, 2);
  },

  applyParam() {
    const item = PARAMS[this.paramIndex];
    const value = this.format(Number(this.selectedParamValue), 2);
    this.sendCommand(item.command + value, () => {
      this.message = item.label + ' 已应用';
      this.refreshStatus();
    });
  },

  syncSelectedParam() {
    const item = PARAMS[this.paramIndex];
    this.selectedParamLabel = item.label;
    if (this.status && this.status[item.key] !== undefined) {
      this.selectedParamValue = this.format(this.status[item.key], 2);
    }
  },

  format(value, digits) {
    const numberValue = Number(value);
    if (isNaN(numberValue)) {
      return '--';
    }
    return numberValue.toFixed(digits);
  },

  clamp(value, min, max) {
    if (value < min) {
      return min;
    }
    if (value > max) {
      return max;
    }
    return value;
  }
};
