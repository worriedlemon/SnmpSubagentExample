# SNMP Subagent

## Preparation

Firstly, be sure your have downloaded several libraries:
+ libsnmp-base
+ libsnmp-dev
+ libsnmp40

and SNMP tools itself with daemon:
+ snmp
+ snmpd

Secondly, for this example to work, you should edit your snmpd.conf file,
which default location is `/etc/snmp/snmpd.conf`. Minimum setup is persented below:

```snmpd.conf
sysServices    72

# using AgentX
master  agentx
agentXSocket /var/agentx/master

# address to call for master agent
agentaddress  udp:localhost:8888

# access control setup
view   systemonly  included   .1.3.6.1.2.1.1
view   systemonly  included   .1.3.6.1.2.1.25.4.2  # hrSWRunTable
view   systemonly  included   .1.3.6.1.2.1.25.5.1  # hrSWRunPerfTable

# setup read-only SNMPv2 community string access
rocommunity  public default -V systemonly

# include other .conf files
includeDir /etc/snmp/snmpd.conf.d
```

As this example overrides default internal tables `hrSWRunTable` and `hrSWRunPerfTable`,
you should disable them on daemon start. This should be done in daemon init settings, defaultly
settled in directory `/lib/systemd/system/snmpd.service` (**for Debian**).

You need to modify `ExecStart` line like

```service
...
ExecStart=/usr/sbin/snmpd -LOw -u Debian-snmp -g Debian-snmp -I -{EXCLUDED_MODULES} -f -M {MIB_FILE_DIRECTORIES} -m {IMPORTED_MIB_FILES}
...
```

where
+ {EXCLUDED_MODULES} - *comma-separated* excluded modules, in our case - internal tables `hrSWRunTable` and `hrSWRunPerfTable`
+ {MIB_FILE_DIRECTORIES} - *colon-separated* directories, where the MIB files are located (in this projects is `{SOURCE_DIR}/mibs`)
+ {IMPORTED_MIB_FILES} - *colon-separed* imported MIB file names without .txt extensions or `ALL`

Example (in this project):
```service
...
ExecStart=/usr/sbin/snmpd -LOw -u Debian-snmp -g Debian-snmp -I -smux,hrSWRunTable,hrSWRunPerfTable -f -M /home/user/snmp_test/mibs -m ALL
...
```

After this, daemon should be reloaded
```bash
systemctl daemon-reload
systemctl restart snmpd
```

## Build

To build this masterpiece you can type

```bash
make all
```

There are several targets:
+ `load/apt` - this target installs required packages using APT
+ `configure` - this target call `configure.sh` script
+ `build` - this target creates CMake directory with *Unix Makefiles*
+ `install` - this target moves program to `./out/` folder
+ `test` - some `snmpget`, `snmpwalk` and `snmptable` testing

## Running

Running a program is simple. Considering that you passed the `install` target:

```
out/snmp_test
```

`sudo` is also can be important, if your agentx socket is owned by root.

If you want to include MIB-files, override some environment variables as follows:

```bash
export MIBDIRS=$(pwd)/mibs
export MIBS=ALL
```

To perform some testing you should run target `test`.

```bash
make test
```

