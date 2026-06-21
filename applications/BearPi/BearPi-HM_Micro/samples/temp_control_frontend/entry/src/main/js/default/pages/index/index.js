import app from '@system.app';

const PARAMS = [
  { label: 'Box loop Kp', command: 'tune box kp ', key: 'box_kp' },
  { label: 'Box loop Ki', command: 'tune box ki ', key: 'box_ki' },
  { label: 'Box loop Kd', command: 'tune box kd ', key: 'box_kd' },
  { label: 'PTC loop Kp', command: 'tune heat kp ', key: 'heat_kp' },
  { label: 'PTC loop Ki', command: 'tune heat ki ', key: 'heat_ki' },
  { label: 'PTC loop Kd', command: 'tune heat kd ', key: 'heat_kd' },
  { label: 'Cool fan Kp', command: 'tune cool kp ', key: 'cool_kp' },
  { label: 'Cool fan Ki', command: 'tune cool ki ', key: 'cool_ki' },
  { label: 'Hysteresis', command: 'tune hys ', key: 'hysteresis_band' },
  { label: 'Warm bias', command: 'tune warmbias ', key: 'warming_bias' },
  { label: 'Heat bias', command: 'tune heatbias ', key: 'heating_bias' }
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
    connectionText: 'Disconnected',
    updatedAt: 'Waiting',
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
    if (!app || !app.tempcontrol) {
      this.connectionText = 'Bridge missing';
      this.message = 'tempcontrol API missing';
      return;
    }
    this.busy = true;
    app.tempcontrol({
      command: command,
      success: (res) => {
        this.busy = false;
        this.connectionText = 'Connected';
        this.message = '';
        if (onSuccess) {
          onSuccess(res.response);
        }
      },
      fail: (err) => {
        this.busy = false;
        this.connectionText = 'Connect failed';
        this.message = err && err.message ? err.message : 'Command failed';
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
        this.message = 'Status parse failed';
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
      this.updatedAt = 'Updated';
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
      this.message = 'Target applied';
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
      this.message = item.label + ' applied';
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
