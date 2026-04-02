# k3s

Installation on the server. The server has a public ip address and then 
is connected via a vpn or other private network to all the nodes.

```bash
curl -sfL https://get.k3s.io | sh -s - server \
  --node-ip 10.0.0.2 \
  --advertise-address 10.0.0.2 \
  --flannel-iface enp7s0 \ # use the correct interface
  --tls-san 100.x.x.x # for another access over tailscale or other service
```

Then get the token from the master `sudo cat /var/lib/rancher/k3s/server/node-token`

On a client install it via 

```bash
curl -sfL https://get.k3s.io | sh -s - agent \
  --server https://10.0.0.2:6443 \
  --token <token> \
  --node-ip 10.0.0.3 \
  --flannel-iface enp7s0
```

To uninstall files are in `/usr/local/bin/`