Please look into the multipath config in `/etc/multipath.conf`
(there needs the config to be adjusted)
e.x.

```conf
defaults {
    user_friendly_names yes
}
blacklist {
    devnode "^sd[a-z0-9]+"
    devnode "^nvme[0-9]+"
    devnode "^dm-[0-9]+"
    wwid ".*"
}
´´´

`sudo systemctl restart multipathd`