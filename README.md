# ZeroTierOneGUI
ZeroTierOneGUI is a free and opensource ZeroTier One GUI for manager the VPNs. This is a wrapper for the zerotier-cli command line tool.

![GUI Screenshot](https://github.com/jimmystelzer/ZeroTierOneGUI/blob/master/artwork/ss_001.jpg?raw=true)

## Current features
- [x] Show current networks in the tray menu with enable/disable selection;
- [x] Add new network from the main window (join).
- [x] Connect/Disconnect from a network with easy click in the main window or in the tray icon menu.
- [x] Show status for each network.
- [x] Uses localized GUI text: pt-BR and en-US.
- [x] Automatic start zerotier service with systemctl (systemd).
- [x] Request "sudo" with pkexec to start service when necessary;
- [x] Remove network from the list of networks (leave).
- [x] Open ZeroTier One Central in the user web browser.
- [x] Automatic configure user token to allow unprivileged user (without sudo needs) to execute zerotier-cli commands.

## Planned features in the roadmap
- [ ] Configuration window.
- [ ] Configuration for enable/disable automatic start zerotier service on app start.
- [ ] Automatic stop zerotier service on quit.
- [ ] Configuration for enable/disable automatic stop zerotier service on app quit.
- [ ] Configuration for where to find token file to support more linux.
- [ ] Configuration for where to find pkexec to allow run systemctl start command for zerotier.
- [ ] Configuration for pkexec alternatives (like gksudo?/gnomesu?/kdesu?/...?).
- [ ] Configuration for systemd alternatives (for non systemd systems).
- [ ] Generate polkit action policy to allow execute systemctrl start/stop for zerotier without password (pkexec) OR a polkit rule to allow execute systemd commands (start/stop/restart) for unit zerotier-one.service without password (no pkexec prefix in the command).
- [ ] Configuration for enable/disable polkit action/rule policy.
