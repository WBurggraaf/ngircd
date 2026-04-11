@echo off
setlocal

cd /d "%~dp0"

if not exist archive mkdir archive
if not exist src mkdir src
if not exist include mkdir include

if exist legacy if not exist archive\upstream-ngircd move legacy archive\upstream-ngircd

if exist archive\upstream-ngircd\src\ngircd if not exist src\server\ngircd (
  if not exist src\server mkdir src\server
  move archive\upstream-ngircd\src\ngircd src\server
)
if exist archive\upstream-ngircd\src\portab if not exist src\support\portab (
  if not exist src\support mkdir src\support
  move archive\upstream-ngircd\src\portab src\support
)
if exist archive\upstream-ngircd\src\tool if not exist src\support\tool (
  if not exist src\support mkdir src\support
  move archive\upstream-ngircd\src\tool src\support
)
if exist archive\upstream-ngircd\src\ipaddr if not exist src\support\ipaddr (
  if not exist src\support mkdir src\support
  move archive\upstream-ngircd\src\ipaddr src\support
)

if exist include\config if not exist include\app\config (
  if not exist include\app mkdir include\app
  move include\config include\app
)
if exist include\host if not exist include\app\host (
  if not exist include\app mkdir include\app
  move include\host include\app
)
if exist include\logging if not exist include\app\logging (
  if not exist include\app mkdir include\app
  move include\logging include\app
)
if exist include\server_app if not exist include\app\server_app (
  if not exist include\app mkdir include\app
  move include\server_app include\app
)
if exist include\core_runtime if not exist include\core\runtime (
  if not exist include\core mkdir include\core
  move include\core_runtime include\core
)
if exist include\net_transport if not exist include\net\transport (
  if not exist include\net mkdir include\net
  move include\net_transport include\net
)
if exist include\resolver if not exist include\net\resolver (
  if not exist include\net mkdir include\net
  move include\resolver include\net
)
if exist include\irc_protocol if not exist include\protocol\irc_protocol (
  if not exist include\protocol mkdir include\protocol
  move include\irc_protocol include\protocol
)
if exist include\command_handlers if not exist include\protocol\command_handlers (
  if not exist include\protocol mkdir include\protocol
  move include\command_handlers include\protocol
)
if exist include\channel_state if not exist include\state\channel_state (
  if not exist include\state mkdir include\state
  move include\channel_state include\state
)
if exist include\client_state if not exist include\state\client_state (
  if not exist include\state mkdir include\state
  move include\client_state include\state
)

echo Component layout migration complete.
