# Glance

Home dashboard. Note the config here is for my dashboard. You can orient yourself and create your own dashboard.

## Docker compose file

```yml
{{#include ./docker-compose.yml}}
```

## `glance.yml` config for the pages

```yml
{{#include ./config/glance.yml}}
```

## `start-page.yml`
The main landing page

```yml
{{#include ./config/start-page.yml}}
```

## `github-repos.yml`
Widget to show the github repos

```yml
{{#include ./config/github-repos-widget.yml}}
```

## `stocks-widget.yml`

```yml
{{#include ./config/stocks-widget.yml}}
```

## `uptime-kuma.yml`

```yml
{{#include ./config/uptime-kuma.yml}}
```

## `.env` file

```env
{{#include ./.env}}
```