# Regression impact review — PR 2 vs `main`

Review only. No source edits.

| | |
|---|---|
| PR | https://github.com/lhj963/simcom_demo/pull/2 (`c1ea55f`, merge `ee3c3b1`) |
| Base | `main` @ `7c8488e` |
| Diff | `main...pr-2`, 16 files, +1518 / −375 |
| Image actually linked | `src/main.c` → `src/src.c` → `src/apps/{mcu,tcp_client,start_tbox}.c` + `src/hal/*` + `src/drivers/*` |
| Not linked | Sibling copies `src/mcu/`, `src/tcp_client/`, `src/start_tbox/` received the same patch. Findings below follow the linked `src/apps` tree. |
| Not run | Simcom SDK image. Host unit test `tests/audit_fixes_test.c` is source evidence only. |

## Verdict

**Breaking** for the original TBOX business. Do not treat PR 2 as protocol-compatible with field MCU commands or with a plaintext JMRH server.

| # | Change | Class |
|---|---|---|
| 1 | `0xA003` `EXE_LINUX_CMD`: `popen` → allowlisted `wget` via `posix_spawnp` | **Breaking** |
| 2 | JMRH decrypt-before-download | **Breaking** for plaintext legacy servers |
| 3 | Socket mutex + `dup` | **Breaking** (overlapping reconnect sticks the channel) |
| 4 | UART reject oversize | **Safe** |
| 5 | Atomic cfg / blind overwrite | **Safe** |
| 6 | Box library path restrict | **Safe** |
| — | SM2 | **Untouched** |

Normal TCCP frames that already fit in the 20KB UART reader, SM2 sign, cfg file bytes, and `/usr/lib/<plain-name>` library installs are unchanged. The three breaking items are live workflows, not theoretical shell abuse.

---

## 1. `EXE_LINUX_CMD` — Breaking

`TCCP_MCU_TO_MPU_EXE_LINUX_CMD_MSG_ID` (`0xA003`) on `main` copies up to 384 bytes into a shell and `popen`s it. `pclose` status is ignored. The ACK is stdout (up to 1024 bytes) or, when stdout is empty, `(%s)execute ok`. After the shell returns, the command **string** is scanned. If it contains `wget`, `-O`, and one of six paths, the matching update runs even when `wget` failed:

- `/data/mcu.bin` → `send_start_mcu_update_msg()` if size > 4096
- `/data/mpu.bin` and `/data/box.bin` → same, plus `is_cmd_mpu_ver_ok()` on the **whole command** (`MPU_V` … `.bin` / `.BIN`)
- `/data/sandian_{mcu,vcu,bms}.zip` → unzip + the existing `system("mv …")` handoff

PR 2 accepts only four tokens, in this order, with no other bytes:

```text
wget -O /data/mcu.bin <url>
wget -O /data/mpu.bin <url>
wget -O /data/box.bin <url>
wget -O /data/sandian_mcu.zip <url>
wget -O /data/sandian_vcu.zip <url>
wget -O /data/sandian_bms.zip <url>
```

`/usr/bin/wget` is accepted as token 0 and then executed as `wget` on `PATH`. The argv is `wget -O <fixed-path> -- <url>`. The URL must be `ftp://`, `http://`, or `https://`, length 8..320, charset `[A-Za-z0-9:/._~?&=%+@-]`. Any `\r`, `\n`, NUL, or other byte below `0x21` except space/tab rejects the payload. `wget` must exit 0. Success ACK is `execute ok\r\n`. Anything else is not executed and is ACKed `execute rejected\r\n`. Version check for mpu/box now sees the URL only.

### Workflows that stop

Anything the MCU (or the platform behind it) sends on `0xA003` that is not that exact line:

- Diagnostics and field ops: `ls`, `cat`, `ps`, `ifconfig`, `ping`, `df`, `date`, `reboot`, `mount`, `chmod`, `cp`, GPIO/`echo`, log collection. Stdout is no longer returned.
- `wget` with a different shape that `main` still upgraded from: `-q`, `-c`, `-T`, `--timeout`, `--no-check-certificate`, `--ftp-user` / `--ftp-password`, URL before `-O`, a shell chain (`&&`, `;`, `|`), quotes, or a trailing CR/LF inside the length-delimited payload.
- Download to any other path, including the in-tree `/data/test_ota.zip` form.
- A URL or password containing a character outside the set (`!`, `*`, space, `#`, `[`, `]`, IPv6 brackets). `:` and `@` are allowed, so `ftp://user:pass@host/file` still parses when the password itself stays inside the set.
- `wget` that writes the file but exits non-zero. `main` ignored the exit status and still set the update flag from the command text.

`0xA012` AT, `0xA00E`/`0xA00F` FTP upload, and the post-download sandian `unzip`/`mv` blocks are different code and are not gated by this allowlist.

### What still works

A canonical four-token `wget -O` to one of those six paths, with a clean URL, and `wget` exit 0. Then the same mcu/mpu/box/三电 follow-up runs. An ACK consumer that only searches for the substring `execute ok` still matches the success ACK. An exact match of `(%s)execute ok` does not.

Device caveat on that surviving path: the new argv inserts `--` before the URL. The old in-tree command does not. Confirm the module `wget` accepts `--`. If it treats `--` as a URL or returns non-zero, all six OTA triggers fail closed (`execute rejected`, no update flag).

---

## 2. JMRH decrypt-before-download — Breaking for plaintext legacy servers

Yes. A plaintext legacy server stops downloading.

On `main`, channel 1 with `FuncSelect_JMRH_SM4` passes the raw TCP buffer to `jmrh_analyze_plat_data`. That function does not look at the JMRH magic and does not decrypt. It searches the bytes for `ftp`, takes the length byte in front of it, then the user and password length bytes, and calls `jmrh_ota_cb`. The callback builds a shell string:

```text
wget -O /data/test_ota.zip ftp://<user>:<pass>@<url-without-ftp://>
```

`system()` return is ignored. `unpack_ota_file` runs anyway. The in-tree vector in `src/apps/mcu/mcu.c` case 19 is this plaintext record (`ftp://222.128.13.25:8121/…zip`, user `users`, password `123456`), not a `03 B6 A1 E4` frame. `tests/audit_fixes_test.c` on the PR asserts that this shape must **not** call the OTA callback.

PR 2 will not scan for `ftp` until `sm4_dec1` succeeds:

- A buffer that contains magic `03 B6 A1 E4` decrypts only the SM4 body (`msglen - 17`, 16-byte aligned). A plaintext body inside that frame is not parsed. There is no gzip inflate. Outbound `jmrh_jiami` is gzip-then-SM4; inbound decrypt does not undo gzip, so a body packed the same way as uplink still will not show `ftp`.
- No magic, length not a multiple of 16, or length > 2048: return, no download. One TCP read is already capped at 2048 (`TCP_THREAD_RECV_BUF_LEN`), so the cap matches a single read and rejects a plaintext command that is not block-aligned.
- No magic and length is a multiple of 16: the whole buffer is decrypted as raw SM4. Decrypting plaintext does not yield the ftp record.

So:

| Server payload | `main` | PR 2 |
|---|---|---|
| Plain length-prefixed `ftp://` record (case 19, current parser) | Downloads | Does not download |
| JMRH frame whose SM4 body **is** that plaintext record | Only if the ciphertext happens to contain `ftp` | Downloads, if the chip/dll is up |
| JMRH frame whose body is gzip(record), same as uplink | Same accidental match only | Decrypts to gzip, then does not see `ftp` |

The `http` branch was already a no-op. It stays a no-op.

Even after a good decrypt, new checks can still drop a server `main` accepted:

- URL must start with `ftp://` (the old search was the three bytes `ftp`).
- User/password charset is `[A-Za-z0-9._-+@%#]`, max 127. `:` `!` `&` `/` space are rejected. The sample `users` / `123456` passes. A colon in the password, which the old `user:pass@host` URL could carry, now fails.
- `safe_ftp_get` uses `wget -O /data/test_ota.zip.partial --ftp-user … --ftp-password … -- <url>`, then `rename`s to `/data/test_ota.zip`, then unpacks. A non-zero `wget` no longer falls through into `unpack_ota_file`. The old use-after-free (`free` the command buffer, then `myreadfile` into it) is gone; `BYSD_BOX` still renames to `/data/box.bin` and calls `update_box_firmware`, otherwise `send_start_mcu_update_msg()`.

---

## 3. Socket mutex / `dup` — Breaking

The linked client is `src/apps/tcp_client/tcp_client.c`. It `poll`s for 3000 ms. The unbuilt `src/tcp_client/tcp_client.c` uses `select` with a 1 s timeout and the same close rule.

What the PR changes:

- One mutex per channel.
- The slot is cleared, then `shutdown(SHUT_RDWR)` + `close` on that fd only.
- Recv and send call `dup` and operate on the duplicate. They close the duplicate, not the slot.
- `sock_close_if_same` closes the slot only when it still holds the fd that was duplicated.

That removes the double-close of one fd number. A late error no longer `close`s a fd that a reconnect already stored over.

The reconnect state machine is not the same in the overlap case.

Normal peer drop still recovers. `read` returns 0, the slot still holds that fd, `sock_close_if_same` clears it, status goes offline, the connect thread sees `sock_get <= 0` and dials again, then sets status online and sends `0xA104`. Channels 7–8 still do not set online on connect; that was already true on `main`.

Overlap does not recover. `poll` holds a dup for up to 3 seconds. The connect thread sleeps 2 seconds around a dial. If the slot is replaced while the dup is still live (server-change flag, or any path that `sock_close`s and then `sock_store`s a new fd before recv handles the old EOF):

1. `shutdown` on the old fd wakes `poll`.
2. `read` on the old dup returns 0.
3. `sock_close_if_same` sees the **new** fd and does not close it.
4. The recv thread still sets connect status to 0 and sends offline.

The new fd stays in the slot. The connect thread only dials when the slot is `<= 0`, so it will not dial. Send and the next `poll` both require connect status, so they ignore the live socket. The channel sits open and idle until some later `sock_close` (another server change) or a reboot. On `main` the EOF path did `close(s_socketfd); s_socketfd = -1`, which could kill a just-installed fd, but the empty slot made the connect thread dial again. The old race recovered. The new race sticks.

`shutdown` is why the dup wakes at all; without it the old connection would stay up until every dup closed. Short writes on the linked client were already failures (`write` length must match). The PR keeps that and only changes which fd is closed.

---

## 4. UART oversize — Safe

The reader buffer is `MAX_UART_RECV_MCU_DATA_BUF_LEN` (20KB). The TCCP length field is 16 bits, so a declared frame can be `content + 7` bytes up to about 64KB. `TCCP_PACK_MIN_LEN` is 7 (`A5 5A` + msgid + length + xor).

On `main`, `tccp_unpack1` waits whenever the bytes in hand are shorter than the declared frame. Once the reader is full it issues a zero-length `read` and keeps calling unpack, which still refuses to consume the sync word. The UART callback never moves again. Later good frames are stuck behind it. A real frame larger than 20KB could not be delivered on `main` either.

PR 2:

- Declared `packlen > 20480` (or `<` minimum): skip one byte past this `A5 5A` and keep scanning.
- A short frame that can still fit is left in place. The new test packs a real frame and also checks that an 8-byte header with content length `0x000A` comes back with `remain == 8`.
- If a prefix in front of a header would make `prefix + packlen` exceed 20KB, the prefix is dropped and the header is kept for the next read. Those prefix bytes are not part of the frame.
- If unpack returns and the reader is still full, one leading byte is dropped so the zero-length read cannot spin.

Business frames already in the field (commands ≤ 384 bytes, param reports, net chunks; file reads use `ONE_BAG_DATA_SIZE` 1024) stay inside the old successful path. A checksum-bad frame that still fits is consumed the same way as before (advance by `packlen`). The behavior change is only the frame that used to wedge the port: it is discarded and the next sync can be found.

---

## 5. Atomic cfg write — Safe

Successful content is the same bytes. The write is safer on power loss.

`mywritefile(create_way == 0)` and `_save_param_func` now write `path.tmp` with `O_CREAT|O_EXCL` mode `0644`, `fsync`, `rename` over the destination, then `fsync` the directory. Appends (`create_way != 0`, the dx83 error log) still use `fopen(..., "ab+")`.

Covered overwrites:

- `/data/sysparam.cfg`, `/data/caprice.cfg`, `/data/caprice_bak.cfg`
- Blind files `/media/card/blind/chN/<unique>.dat` (new name each time)
- `/data/sys_status.dat`, hostapd/dnsmasq confs, `/data/dbg_tip.txt`, `/data/mcu_e_param.dat`
- `/usr/bin/wlan0_ap.sh`, which is still followed by `chmod 777`

Box cfg install used to `rm -rf /data/sysparam.cfg` and then `DBG_SendDataEx`, and `dbg_send` opens with `"ab+"`. After the delete, that append created a file whose bytes were the new blob. The replacement write stores that same blob. Callers of `save_sysparam_func` ignore the return code. On success they still see the struct bytes they passed. On failure the previous file remains; `main` could leave a truncated file because `fopen("wb+")` truncates before `fwrite`.

Caveats that do not change a successful read:

- An empty cfg payload (`pdat == NULL` or `datlen == 0`) no longer deletes `/data/sysparam.cfg`. `main` deleted first, and `dbg_send` then wrote nothing.
- A new file is mode `0644`. `fopen` kept the old inode mode. The script path chmods after the write. Shared libraries and cfg files are not executed via `execve`.
- Each blind record and each cfg save now `fsync`s. That can stall the caller on a slow card. It does not change the bytes.
- A crash can leave `path.tmp`. A blind-folder listing that sends every file could upload that leftover once. A finished write does not leave the temp name.

---

## 6. Box install path — Safe

Legitimate installs were already only under `/usr/lib`. The format field is a file name (`lib1文件名`, X bytes), not a destination directory. `lib_firmware_proc` always did `sprintf(fn, "/usr/lib/%s", name)`, then `system("rm -rf %s")` and a write. The packer sample names are `libsm9.so`, `libutil.so`, `libpoc.so`. The Wi-Fi script loads `/usr/lib/rtl8189es.ko`, also a single component. There is no original API that installs to `/usr/bin`, `/lib`, or a path the package chooses, except by putting `../` or shell metacharacters in the name.

PR 2 keeps the `/usr/lib/` prefix and accepts one component: length 1..64, not starting with `.`, no `..`, charset `[A-Za-z0-9._-+]`. `+` is included, so a `libstdc++` style name still passes. CRC32 is still required before unpack returns 0. Names and offsets that walk off the image return 4 or 6 instead of copying past the buffer. A well-formed package with those sample names still installs, and `update_box_firmware` still reboots only when unpack returns 0.

A name with a slash (`modules/foo.ko`), a space, or a shell character is rejected. That could not be a supported path; it was how `rm -rf` was aimed. `fn_len > 64` now fails the unpack before the mcu/mpu/cfg callbacks. `main` truncated such a name to 64 bytes with no guaranteed NUL and continued.

Reliability note, same hole as `rm` then write: `box_install_library` `unlink`s the existing file and then calls `durable_write_file`. If the new write fails, the old library is already gone. Mode of the new file is `0644`, which `dlopen` can map.

---

## SM2 — untouched

Confirmed. `git diff main...pr-2 -G 'SM2|sm2_'` is empty.

Not in the diff: `DX83_API.h` SM2 agreement / id / sign prototypes, `SM2_sign` / `SM2_sign_UID` / `SM2_getPublicKey` / `SM2_getPublicKey2` in `src/drivers/dx83/dx83_test.c`, `src/hal/hj1239/hj1239.c`, and the MCU handlers `TCCP_MCU_TO_MPU_GET_SM2_SIGN_MSG_ID` (`0xA009`) / `TCCP_MPU_TO_MCU_SM2_SIGN_DATA_ARRIVE_MSG_ID` (`0xA10A`).

The only `dx83` edit adds `sm4_dec1`, a chunked `SM4_Decrypt` (224-byte blocks, same shape as `sm4_enc1`). That is SM4, used by the new JMRH decrypt. It does not call or wrap an SM2 entry.

---

## 原功能影响

总判：**Breaking**。不能把 PR 2 当成对现网 TBOX 无感的加固。SM2、20KB 以内的正常串口帧、配置文件成功写入后的内容、以及 `/usr/lib` 下普通库名安装都还在。下面三件事会改变已经在跑的业务。

1. **远程命令。** `0xA003` 不再是 shell。MCU 下发的诊断、重启、带 `-q`/`--ftp-user`/其它参数的 `wget`、下到其它路径、命令末尾带 `\r\n`，一律不执行，应答 `execute rejected`。只有恰好 `wget -O` 加六个固定文件之一，且 `wget` 退出码为 0，才会继续原来的 MCU / MPU / box / 三电升级。成功应答从 `(%s)execute ok` 或命令 stdout 变成 `execute ok\r\n`。
2. **JMRH 升级。** 现网如果按现在的解析发明文（长度字节 + `ftp://` + 账号 + 密码，仓库 case 19 就是这样），升级不再开始。必须先 SM4 解密成功，解密结果仍是这条明文记录，账号密码还得落在新字符集里。按上行那样 gzip 再 SM4 的包，解密后是 gzip，同样找不到 `ftp`。下载失败也不再拿旧的 `/data/test_ota.zip` 去验签。
3. **TCP 重连。** 对端正常断开、槽里还是那一个 fd 时，仍会关掉并重拨。改服务器或其它“先关掉再拨通”如果和 recv 的 3 秒 `poll` 叠在一起，旧连接的 EOF 会把状态打成离线，新 socket 却留在槽里。连接线程看到 fd 仍大于 0，不再拨号，这条链路停在那里，直到再次触发 `sock_close` 或重启。
4. **串口。** 声明长度超过 20KB 的帧以前会把接收卡死，现在丢掉并继续对同步。装得下的短帧会留到下一次读。正常业务帧没有变化。
5. **配置和盲区。** 成功时字节与原来一致。掉电时旧的 `sysparam.cfg` / `caprice.cfg` 还在，不再先截成空文件。空的 cfg 载荷不再删除 `/data/sysparam.cfg`。
6. **Box 库。** 本来就只装到 `/usr/lib/文件名`。`libsm9.so`、`libutil.so`、`libpoc.so` 这种名字可以装。没有“合法地装到 `/usr/lib` 以外”的路径；带 `/`、`..` 或 shell 字符的名字会被拒绝。
7. **SM2。** 签名、公钥、协商以及 `0xA009` / `0xA10A` 没有改动。
