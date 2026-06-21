# PID Temperature Control HAP

This directory contains the ACE Lite JS source for the BearPi-HM Micro PID
temperature control panel.

## Runtime contract

The system image must include the `@system.app.tempcontrol` native API and the
`control` backend process. The app talks to the backend through:

```js
app.tempcontrol({
  command: "get_status",
  success: function (res) {
    const status = JSON.parse(res.response);
  }
});
```

Supported commands are the backend protocol commands exposed by `control`:

- `get_status`
- `tune target <value>`
- `tune hys <value>`
- `tune warmbias <value>`
- `tune heatbias <value>`
- `tune box kp|ki|kd <value>`
- `tune heat kp|ki|kd <value>`
- `tune cool kp|ki <value>`

## Build and install

Build this folder with the OpenHarmony/DevEco JS toolchain as an ACE Lite JS
application, then install the generated HAP on the board:

```sh
./bm set -s disable
./bm set -d enable
./bm install -p TempControl_1.0.0.hap
```

The `control` backend should be running on the same board and listening on
`127.0.0.1:5000`.
