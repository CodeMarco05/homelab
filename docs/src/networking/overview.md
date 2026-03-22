# Networkign

My network is split into my remote services and the application I host at my home. All is connected
via Tailscale. But i plan to switch Netbird.

For some use cases it is really useful to use _Nginx Proxy Manager_. In the portmapping the
**127.0.0.1** is that it only maps to the local interface. So from outside it is not forwoarded and
takes no security issue. You can forwoard it via `ssh -L <port>:localhost:<remote-port> <ssh-host>`

```yml
services:
  nginx-proxy-manager:
    image: "jc21/nginx-proxy-manager:latest"
    restart: always
    container_name: nginx-proxy-manager
    ports:
      - "127.0.0.1:80:80"
      - "127.0.0.1:81:81"
      - "127.0.0.1:443:443"
    networks:
      - proxy
    volumes:
      - ./data:/data
      - ./letsencrypt:/etc/letsencrypt

networks:
  proxy:
    name: proxy
    driver: bridge
```


## Traefik

For the rest i only use traefik. The setup is split into two sections

For servers with a public IP adress you can use the certresolver via http and behind private
networks etc. you can use an alternative resolver over a dns challenge.
