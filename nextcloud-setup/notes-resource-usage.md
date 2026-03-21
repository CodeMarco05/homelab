# Nextcloud AIO — Performance Settings

## How it works

Nextcloud AIO uses its own environment variables which get passed down to the actual
Nextcloud PHP-FPM container. So `NEXTCLOUD_MEMORY_LIMIT` in the compose file becomes
`PHP_MEMORY_LIMIT` inside the container.

> ⚠️ **Important:** `docker exec ... php -r "echo ini_get('memory_limit');"` shows the
> **CLI** limit — not the actual FPM limit. Always verify with:
> ```bash
> docker exec -it nextcloud-aio-nextcloud env | grep MEMORY
> ```

---

## Settings in docker-compose.yml

```yaml
NEXTCLOUD_MEMORY_LIMIT: 6144M   # PHP memory per process (FPM)
NEXTCLOUD_UPLOAD_LIMIT: 32G     # Max file upload size
NEXTCLOUD_MAX_TIME: 18000       # Max execution time in seconds (18000 = 5 hours)
```

---

## NEXTCLOUD_MEMORY_LIMIT

Controls how much RAM each PHP-FPM process can use.

| Server RAM | Empfehlung |
|---|---|
| 4 GB | `512M` (default) |
| 8 GB | `1024M` |
| 16 GB | `2048M` |
| 32 GB+ | `4096M` – `6144M` |

Verify it is correctly set inside the container:
```bash
docker exec -it nextcloud-aio-nextcloud env | grep MEMORY
# Should show: PHP_MEMORY_LIMIT=6144M
```

---

## NEXTCLOUD_UPLOAD_LIMIT

Controls the maximum size of a single file upload.

```yaml
NEXTCLOUD_UPLOAD_LIMIT: 32G
```

- Default is `16G`
- Set higher if you upload large video files or archives
- This sets both the PHP `upload_max_filesize` and `post_max_size` inside the container

---

## NEXTCLOUD_MAX_TIME

Controls how long a PHP process can run before timing out (in seconds).

```yaml
NEXTCLOUD_MAX_TIME: 18000  # 5 hours
```

- Default is `3600` (1 hour)
- Important for large file uploads on slow connections
- Also affects background jobs and long-running operations

---

## Applying changes

After editing `docker-compose.yml`, restart everything so AIO recreates the containers
with the new values:

```bash
docker compose down
docker compose up -d
```

Then verify all three settings inside the container:
```bash
docker exec -it nextcloud-aio-nextcloud env | grep -E "MEMORY|UPLOAD|TIME"
```
