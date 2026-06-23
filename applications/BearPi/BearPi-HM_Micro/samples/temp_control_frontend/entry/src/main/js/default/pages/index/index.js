import app from '@system.app';

export default {
  data: {
    currentTemperature: '--',
    appliedTarget: '--',
    pendingTarget: 40,
    pendingTargetText: '40.0',
    targetDirty: false,
    targetHint: '',
    boxHumidity: '--',
    boxPressure: '--',
    ptcTemperature: '--',
    ptcTargetTemperature: '--',
    currentPwm: '--',
    controlState: '--',
    ptcState: '--',
    hysteresisBand: '--',
    connectionText: '未连接',
    updatedAt: '等待中',
    message: '',
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
    if (!app || !app.tempcontrol) {
      this.connectionText = '桥接缺失';
      this.message = 'tempcontrol 接口缺失';
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
        this.message = '状态解析失败';
        return;
      }
      this.status = next;
      this.currentTemperature = this.format(next.current_temperature, 1);
      this.appliedTarget = this.format(next.target_temperature, 1);
      if (!this.targetDirty) {
        this.pendingTarget = Number(next.target_temperature);
        this.pendingTargetText = this.appliedTarget;
        this.targetHint = '';
      }
      this.boxHumidity = this.format(next.box_humidity, 1);
      this.boxPressure = this.format(next.box_pressure, 0);
      this.ptcTemperature = this.format(next.current_ptc_temperature, 1);
      this.ptcTargetTemperature = this.format(next.ptc_target_temperature, 1);
      this.currentPwm = this.format(Number(next.current_pwm) * 100, 0);
      this.controlState = this.mapControlState(next.control_state);
      this.ptcState = this.mapPtcState(next.ptc_state);
      this.hysteresisBand = this.format(next.hysteresis_band, 1);
      this.updatedAt = '已更新';
    });
  },

  targetDown() {
    this.pendingTarget = this.clamp(Number(this.pendingTarget) - 1, 20, 90);
    this.pendingTargetText = this.format(this.pendingTarget, 1);
    this.targetDirty = true;
    this.targetHint = '待设定';
  },

  targetUp() {
    this.pendingTarget = this.clamp(Number(this.pendingTarget) + 1, 20, 90);
    this.pendingTargetText = this.format(this.pendingTarget, 1);
    this.targetDirty = true;
    this.targetHint = '待设定';
  },

  applyTarget() {
    const value = this.format(this.pendingTarget, 1);
    this.sendCommand('tune target ' + value, () => {
      this.message = '目标已设定';
      this.targetDirty = false;
      this.targetHint = '';
      this.refreshStatus();
    });
  },

  mapControlState(state) {
    if (state === 'HEATING') {
      return '加热';
    }
    if (state === 'WARMING') {
      return '保温';
    }
    if (state === 'COOLING') {
      return '制冷';
    }
    return state || '--';
  },

  mapPtcState(state) {
    if (state === 'ON') {
      return '开';
    }
    if (state === 'OFF') {
      return '关';
    }
    return state || '--';
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
