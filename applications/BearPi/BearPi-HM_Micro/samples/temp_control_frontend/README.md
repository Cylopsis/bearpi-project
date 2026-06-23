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

This app is built through the OpenHarmony HAP build templates, not by manually
zipping the source tree. The build depends on the JS HAP toolchain under:

```text
prebuilts/sdk/js-loader/build-tools/ace-loader
prebuilts/build-tools/common/nodejs/node-v12.18.4-<host>-x64
prebuilts/build-tools/common/restool/restool
```

Build only this HAP target from the repository root:

```sh
hb build -T //applications/BearPi/BearPi-HM_Micro/samples/temp_control_frontend:temp_control_hap
```

If the JS HAP toolchain is missing, prepare it first from the Linux build
environment:

```sh
bash scripts/prepare_tempcontrol_hap_env.sh
```

The generated HAP path is:

```text
out/<product>/system/internal/TempControl.hap
```

Copy it to the board storage as `/storage/TempControl.hap`, then install it from
the board shell:

```sh
cd /vendor/hap_tools
./bm set -s disable
./bm set -d enable
./bm uninstall -n com.bearpi.tempcontrol
./bm install -p /storage/TempControl.hap
./bm dump -n com.bearpi.tempcontrol
```

The `control` backend should be running on the same board and listening on
`127.0.0.1:5000`.

Launch the app from the Launcher icon. The BearPi image used during bring-up
does not provide the standard `aa` command-line launcher.
