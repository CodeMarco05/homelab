# Nextcloud AIO Setup Reference

## Docker Compose
```yaml
environment:
  APACHE_PORT: 11000
  APACHE_IP_BINDING: 0.0.0.0
  SKIP_DOMAIN_VALIDATION: true
ports:
  - "8080:8080"
```

## Docker Commands
```bash
# start
docker compose up -d

# stop
docker compose down

# logs
docker logs nextcloud-aio-mastercontainer

# remove everything
docker stop $(docker ps -aq) && \
docker rm $(docker ps -aq) && \
docker volume rm $(docker volume ls -q) && \
docker network prune -f
```

## Test Nextcloud
```bash
curl -v http://localhost:11000
curl -v http://10.10.1.133:11000
```

## Cloudflare Tunnel

- Service: `https://10.10.1.99:443`
- No TLS Verify: ✅
- Upload limit: Cloudflare Dashboard → Network → disable

## NPM Proxy Host

- Domain: `nextcloud.marco-brandt.com`
- Forward: `10.10.1.133:11000`
- SSL: Let's Encrypt via Cloudflare DNS challenge

Advanced config:
```nginx
proxy_set_header X-Real-IP $remote_addr;
proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
proxy_set_header X-Forwarded-Proto $scheme;
proxy_set_header X-Forwarded-Host $host;
proxy_hide_header Upgrade;
```

## OPNsense Static DHCP Lease

Services → DHCPv4 → LAN → Add
- MAC: `bc:24:11:7b:c5:62`
- IP: `10.10.1.133`

## OPNsense Split DNS

Services → Unbound DNS → Host Overrides → Add
- Host: `nextcloud`
- Domain: `marco-brandt.com`
- IP: `10.10.1.99`

## DNS Lookup
```bash
nslookup nextcloud.marco-brandt.com            # your machine
nslookup nextcloud.marco-brandt.com 8.8.8.8    # internet
nslookup nextcloud.marco-brandt.com 10.10.1.1  # local
```

## Mac DNS

System Settings → Network → WiFi → Details → DNS
- Keep only: `10.10.1.1`
```bash
# flush DNS cache
sudo dscacheutil -flushcache && sudo killall -HUP mDNSResponder
```

## Network Commands
```bash
# show interfaces
ip link show

# show IP
ip addr show ens18

# renew DHCP lease
sudo systemctl restart systemd-networkd
```
