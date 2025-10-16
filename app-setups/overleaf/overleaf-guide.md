# Overleaf Self-Hosted Complete Guide

## Table of Contents
1. [Common Issues & Fixes](#common-issues--fixes)
2. [Configuration](#configuration)
3. [User Management](#user-management)
4. [Backup & Restore](#backup--restore)
5. [Logs & Debugging](#logs--debugging)
6. [Maintenance](#maintenance)

---

## Common Issues & Fixes

### Issue 1: Compilation Fails - Docker Socket Error

**Error Message:**
```
"error":"connect ENOENT /var/run/docker.sock"
```

**Cause:** CLSI is trying to use Docker-in-Docker sandboxed compilation but can't access the Docker socket.

**Solution:**

Edit your `docker-compose.yml` and ensure these settings in the environment section:

```yaml
environment:
    SANDBOXED_COMPILES: 'false'
```

**Remove or comment out these lines if they exist:**
```yaml
# SANDBOXED_COMPILES_HOST_DIR_COMPILES: '/home/user/sharelatex_data/data/compiles'
# SANDBOXED_COMPILES_HOST_DIR_OUTPUT: '/home/user/sharelatex_data/data/output'
# DOCKER_RUNNER: 'true'
# SANDBOXED_COMPILES_SIBLING_CONTAINERS: 'true'
```

Apply changes:
```bash
cd ~/overleaf
docker-compose down
docker-compose up -d
```

### Issue 2: MongoDB Requires AVX CPU Support

**Error Message:**
```
MongoDB 5.0+ requires a CPU with AVX support
```

**Solution Option 1 - Downgrade MongoDB (Recommended):**

Edit `docker-compose.yml`:
```yaml
mongo:
    image: mongo:4.4  # Change from mongo:5.0 or mongo:6.0
```

**Solution Option 2 - Enable AVX in Proxmox VM:**

1. Check if physical CPU supports AVX:
   ```bash
   grep avx /proc/cpuinfo
   ```

2. If yes, change VM CPU type in Proxmox:
   - Web UI: VM → Hardware → Processors → Edit → Type: `host`
   - OR via command line:
     ```bash
     qm set <VM_ID> --cpu host
     ```

3. Restart the VM:
   ```bash
   qm stop <VM_ID>
   qm start <VM_ID>
   ```

4. Verify inside VM:
   ```bash
   grep avx /proc/cpuinfo
   ```

### Issue 3: Compilation Timeout

**Symptom:** Large documents timeout during compilation.

**Solution:**

Add to `docker-compose.yml` environment section:
```yaml
environment:
    SHARELATEX_COMPILE_TIMEOUT: 180  # 180 seconds
```

### Issue 4: Missing LaTeX Packages

**Symptom:** Compilation fails with "File X.sty not found"

**Solution:**

```bash
# Enter container
docker exec -it sharelatex bash

# Update TeX Live manager
tlmgr update --self

# Install specific package
tlmgr install <package-name>

# OR install everything (large download ~5GB)
tlmgr install scheme-full
```

---

## Configuration

### Minimal Working docker-compose.yml

```yaml
services:
    sharelatex:
        restart: always
        image: sharelatex/sharelatex
        container_name: sharelatex
        depends_on:
            mongo:
                condition: service_healthy
            redis:
                condition: service_started
        ports:
            - 8000:80
        stop_grace_period: 60s
        volumes:
            - ~/sharelatex_data:/var/lib/overleaf
        environment:
            OVERLEAF_APP_NAME: Overleaf Community Edition
            OVERLEAF_MONGO_URL: mongodb://mongo/sharelatex
            OVERLEAF_REDIS_HOST: redis
            REDIS_HOST: redis
            ENABLED_LINKED_FILE_TYPES: 'project_file,project_output_file'
            ENABLE_CONVERSIONS: 'true'
            EMAIL_CONFIRMATION_DISABLED: 'true'
            SANDBOXED_COMPILES: 'false'
            SHARELATEX_COMPILE_TIMEOUT: 180

    mongo:
        restart: always
        image: mongo:4.4  # or mongo:6.0 if CPU supports AVX
        container_name: mongo
        command: '--replSet overleaf'
        volumes:
            - ~/mongo_data:/data/db
            - ./bin/shared/mongodb-init-replica-set.js:/docker-entrypoint-initdb.d/mongodb-init-replica-set.js
        environment:
            MONGO_INITDB_DATABASE: sharelatex
        extra_hosts:
            - mongo:127.0.0.1
        healthcheck:
            test: echo 'db.stats().ok' | mongosh localhost:27017/test --quiet
            interval: 10s
            timeout: 10s
            retries: 5

    redis:
        restart: always
        image: redis:6.2
        container_name: redis
        volumes:
            - ~/redis_data:/data
```

### Important Data Locations

| Type | Location in Container | Location on Host |
|------|----------------------|------------------|
| Projects & Files | `/var/lib/overleaf/data` | `~/sharelatex_data/data` |
| Compile Cache | `/var/lib/overleaf/data/compiles` | `~/sharelatex_data/data/compiles` |
| MongoDB Database | `/data/db` | `~/mongo_data` |
| Redis Data | `/data` | `~/redis_data` |
| Docker Compose | N/A | `~/overleaf/docker-compose.yml` |

---

## User Management

### Create Admin User (First Time Setup)

When you first install Overleaf, you need to create an admin user:

```bash
# Enter the container
docker exec -it sharelatex bash

# Create admin user
cd /overleaf/services/web
node modules/server-ce-scripts/scripts/create-admin-user.js admin@example.com
```

You'll be prompted to enter a password. Save this password securely!

### Create Regular Users

**Method 1: Via Command Line**

```bash
# Enter container
docker exec -it sharelatex bash

# Create user
cd /overleaf/services/web
node modules/server-ce-scripts/scripts/create-user.js \
  --email user@example.com \
  --password yourpassword \
  --first-name John \
  --last-name Doe
```

**Method 2: Via Web Interface (if registration is enabled)**

Users can register themselves at: `http://your-server:8000/register`

To enable/disable registration, edit `docker-compose.yml`:

```yaml
environment:
    # Allow open registration
    OVERLEAF_ALLOW_PUBLIC_ACCESS: 'true'
    
    # OR disable registration (users must be created manually)
    # OVERLEAF_ALLOW_PUBLIC_ACCESS: 'false'
```

### Grant Admin Rights to Existing User

```bash
# Enter container
docker exec -it sharelatex bash

# Make user admin
cd /overleaf/services/web
node modules/server-ce-scripts/scripts/set-admin-privilege.js user@example.com true
```

### Remove Admin Rights

```bash
docker exec -it sharelatex bash
cd /overleaf/services/web
node modules/server-ce-scripts/scripts/set-admin-privilege.js user@example.com false
```

### List All Users

```bash
# Enter MongoDB container
docker exec -it mongo mongosh

# Switch to sharelatex database
use sharelatex

# List all users
db.users.find({}, {email: 1, isAdmin: 1, first_name: 1, last_name: 1, _id: 0}).pretty()

# Count total users
db.users.count()

# List only admins
db.users.find({isAdmin: true}, {email: 1, first_name: 1, last_name: 1}).pretty()

# Exit mongosh
exit
```

### Delete a User

⚠️ **WARNING: This permanently deletes the user and their projects!**

```bash
# First, backup!
~/backup_overleaf.sh

# Enter MongoDB
docker exec -it mongo mongosh

use sharelatex

# Find user ID first
db.users.find({email: "user@example.com"}, {_id: 1, email: 1})

# Delete user (replace USER_ID with actual ID)
db.users.deleteOne({email: "user@example.com"})

exit
```

### Reset User Password

```bash
# Enter container
docker exec -it sharelatex bash

# Reset password
cd /overleaf/services/web
node modules/server-ce-scripts/scripts/reset-password.js user@example.com newpassword
```

### Verify User Email Manually

If email confirmation is enabled but not working:

```bash
# Enter MongoDB
docker exec -it mongo mongosh

use sharelatex

# Confirm user email
db.users.updateOne(
  {email: "user@example.com"},
  {$set: {emails: [{email: "user@example.com", confirmedAt: new Date()}]}}
)

exit
```

### Check User Details

```bash
# Enter MongoDB
docker exec -it mongo mongosh

use sharelatex

# View specific user details
db.users.findOne({email: "user@example.com"})

# View user's projects
db.projects.find({owner_ref: ObjectId("USER_ID_HERE")}, {name: 1, lastUpdated: 1})

exit
```

### Disable Email Confirmation Requirement

Edit `docker-compose.yml`:

```yaml
environment:
    EMAIL_CONFIRMATION_DISABLED: 'true'
```

Then restart:
```bash
cd ~/overleaf
docker-compose restart sharelatex
```

### User Account Management Script

Create `~/manage_overleaf_user.sh`:

```bash
#!/bin/bash

echo "Overleaf User Management"
echo "========================"
echo "1. Create user"
echo "2. Create admin user"
echo "3. Grant admin rights"
echo "4. Reset password"
echo "5. List all users"
echo "6. Delete user"
echo "7. Exit"
echo
read -p "Choose option: " option

case $option in
    1)
        read -p "Email: " email
        read -sp "Password: " password
        echo
        read -p "First Name: " firstname
        read -p "Last Name: " lastname
        docker exec -it sharelatex bash -c "cd /overleaf/services/web && node modules/server-ce-scripts/scripts/create-user.js --email $email --password $password --first-name '$firstname' --last-name '$lastname'"
        ;;
    2)
        read -p "Admin Email: " email
        docker exec -it sharelatex bash -c "cd /overleaf/services/web && node modules/server-ce-scripts/scripts/create-admin-user.js $email"
        ;;
    3)
        read -p "User Email: " email
        docker exec -it sharelatex bash -c "cd /overleaf/services/web && node modules/server-ce-scripts/scripts/set-admin-privilege.js $email true"
        echo "Admin rights granted to $email"
        ;;
    4)
        read -p "Email: " email
        read -sp "New Password: " password
        echo
        docker exec -it sharelatex bash -c "cd /overleaf/services/web && node modules/server-ce-scripts/scripts/reset-password.js $email $password"
        echo "Password reset for $email"
        ;;
    5)
        docker exec -it mongo mongosh --quiet --eval "use sharelatex; db.users.find({}, {email: 1, isAdmin: 1, first_name: 1, last_name: 1, _id: 0}).forEach(printjson)"
        ;;
    6)
        read -p "Email to delete: " email
        read -p "Are you sure? This will delete ALL their projects! (yes/no): " confirm
        if [ "$confirm" == "yes" ]; then
            docker exec -it mongo mongosh --quiet --eval "use sharelatex; db.users.deleteOne({email: '$email'})"
            echo "User $email deleted"
        else
            echo "Cancelled"
        fi
        ;;
    7)
        exit 0
        ;;
    *)
        echo "Invalid option"
        ;;
esac
```

Make it executable:
```bash
chmod +x ~/manage_overleaf_user.sh
```

Run it:
```bash
~/manage_overleaf_user.sh
```

### Bulk User Creation

Create a CSV file `users.csv`:
```
email,password,firstname,lastname,admin
user1@example.com,pass123,John,Doe,false
user2@example.com,pass456,Jane,Smith,true
user3@example.com,pass789,Bob,Jones,false
```

Create script `~/bulk_create_users.sh`:

```bash
#!/bin/bash

CSV_FILE="users.csv"

if [ ! -f "$CSV_FILE" ]; then
    echo "Error: $CSV_FILE not found"
    exit 1
fi

# Skip header line
tail -n +2 "$CSV_FILE" | while IFS=',' read -r email password firstname lastname admin
do
    echo "Creating user: $email"
    
    docker exec sharelatex bash -c "cd /overleaf/services/web && \
        node modules/server-ce-scripts/scripts/create-user.js \
        --email '$email' \
        --password '$password' \
        --first-name '$firstname' \
        --last-name '$lastname'" 2>/dev/null
    
    if [ "$admin" == "true" ]; then
        echo "  Granting admin rights..."
        docker exec sharelatex bash -c "cd /overleaf/services/web && \
            node modules/server-ce-scripts/scripts/set-admin-privilege.js '$email' true" 2>/dev/null
    fi
    
    echo "  ✓ Created: $email"
done

echo "Bulk user creation completed"
```

### User Quotas and Limits

Community Edition doesn't have built-in user quotas, but you can monitor usage:

**Check user's project count:**
```bash
docker exec -it mongo mongosh --quiet --eval "
use sharelatex;
db.users.aggregate([
  {
    \$lookup: {
      from: 'projects',
      localField: '_id',
      foreignField: 'owner_ref',
      as: 'projects'
    }
  },
  {
    \$project: {
      email: 1,
      projectCount: { \$size: '\$projects' }
    }
  },
  {
    \$sort: { projectCount: -1 }
  }
]).forEach(printjson)
"
```

**Check total disk usage per user:**
```bash
docker exec sharelatex bash -c "
cd /var/lib/overleaf/data
for user_dir in */; do
    if [ -d \"\$user_dir\" ]; then
        size=\$(du -sh \"\$user_dir\" 2>/dev/null | cut -f1)
        echo \"\$user_dir: \$size\"
    fi
done | sort -h -k2
"
```

### LDAP Integration (Optional)

For enterprise environments, you can integrate LDAP/Active Directory:

Edit `docker-compose.yml`:

```yaml
environment:
    # LDAP Configuration
    OVERLEAF_LDAP_URL: 'ldap://ldap.example.com:389'
    OVERLEAF_LDAP_SEARCH_BASE: 'ou=users,dc=example,dc=com'
    OVERLEAF_LDAP_SEARCH_FILTER: '(uid={{username}})'
    OVERLEAF_LDAP_BIND_DN: 'cn=admin,dc=example,dc=com'
    OVERLEAF_LDAP_BIND_CREDENTIALS: 'your-ldap-password'
    OVERLEAF_LDAP_EMAIL_ATT: 'mail'
    OVERLEAF_LDAP_NAME_ATT: 'cn'
    OVERLEAF_LDAP_LAST_NAME_ATT: 'sn'
    OVERLEAF_LDAP_UPDATE_USER_DETAILS_ON_LOGIN: 'true'
```

### Best Practices

1. **Always create at least one admin user** during initial setup
2. **Disable email confirmation** if you don't have SMTP configured: `EMAIL_CONFIRMATION_DISABLED: 'true'`
3. **Disable public registration** for private instances: `OVERLEAF_ALLOW_PUBLIC_ACCESS: 'false'`
4. **Backup before deleting users** - deletions are permanent!
5. **Use strong passwords** for admin accounts
6. **Regularly audit user list** to remove inactive accounts
7. **Monitor disk usage** per user to prevent abuse

### Troubleshooting User Issues

**User can't log in:**
```bash
# Check if user exists
docker exec -it mongo mongosh --quiet --eval "use sharelatex; db.users.findOne({email: 'user@example.com'}, {email: 1, hashedPassword: 1})"

# Verify email is confirmed (if confirmation is enabled)
docker exec -it mongo mongosh --quiet --eval "use sharelatex; db.users.findOne({email: 'user@example.com'}, {emails: 1})"

# Reset password
docker exec sharelatex bash -c "cd /overleaf/services/web && node modules/server-ce-scripts/scripts/reset-password.js user@example.com newpassword"
```

**User forgot password (no email configured):**
```bash
# Admin must reset it manually
docker exec sharelatex bash -c "cd /overleaf/services/web && node modules/server-ce-scripts/scripts/reset-password.js user@example.com temporary123"
# Tell user to log in with temporary123 and change it
```

---

## Backup & Restore

### Quick Backup (With Downtime)

```bash
# Stop containers
cd ~/overleaf
docker-compose down

# Create timestamped backup
BACKUP_DIR=~/overleaf_backup_$(date +%Y%m%d_%H%M%S)
mkdir -p $BACKUP_DIR

# Backup all data
cp -r ~/sharelatex_data $BACKUP_DIR/
cp -r ~/mongo_data $BACKUP_DIR/
cp -r ~/redis_data $BACKUP_DIR/
cp ~/overleaf/docker-compose.yml $BACKUP_DIR/

# Compress
tar -czf ~/overleaf_backup_$(date +%Y%m%d_%H%M%S).tar.gz -C ~/ $(basename $BACKUP_DIR)

# Start containers
cd ~/overleaf
docker-compose up -d

echo "Backup saved to: ~/overleaf_backup_$(date +%Y%m%d_%H%M%S).tar.gz"
```

### Hot Backup (Without Downtime)

```bash
# Create backup directory
BACKUP_DIR=~/overleaf_backup_$(date +%Y%m%d_%H%M%S)
mkdir -p $BACKUP_DIR

# Backup using rsync
rsync -av ~/sharelatex_data/ $BACKUP_DIR/sharelatex_data/
rsync -av ~/mongo_data/ $BACKUP_DIR/mongo_data/
rsync -av ~/redis_data/ $BACKUP_DIR/redis_data/
cp ~/overleaf/docker-compose.yml $BACKUP_DIR/

# MongoDB dump (better for database consistency)
docker exec mongo mongodump --out=/data/db/backup_temp
cp -r ~/mongo_data/backup_temp $BACKUP_DIR/mongo_backup
docker exec mongo rm -rf /data/db/backup_temp

# Compress
tar -czf ~/overleaf_backup_$(date +%Y%m%d_%H%M%S).tar.gz -C ~/ $(basename $BACKUP_DIR)
rm -rf $BACKUP_DIR

echo "Hot backup completed"
```

### Automated Backup Script

Create file `~/backup_overleaf.sh`:

```bash
#!/bin/bash

# Configuration
BACKUP_BASE=~/overleaf_backups
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
BACKUP_DIR=$BACKUP_BASE/backup_$TIMESTAMP
KEEP_BACKUPS=7  # Number of backups to keep

# Create backup directory
mkdir -p $BACKUP_DIR

echo "[$TIMESTAMP] Starting Overleaf backup..."

# Backup Overleaf data
echo "Backing up Overleaf data..."
rsync -a --quiet ~/sharelatex_data/ $BACKUP_DIR/sharelatex_data/

# Backup MongoDB with mongodump
echo "Backing up MongoDB..."
docker exec mongo mongodump --quiet --out=/data/db/backup_$TIMESTAMP 2>/dev/null
if [ -d ~/mongo_data/backup_$TIMESTAMP ]; then
    cp -r ~/mongo_data/backup_$TIMESTAMP $BACKUP_DIR/mongo_backup
    docker exec mongo rm -rf /data/db/backup_$TIMESTAMP
fi

# Backup Redis
echo "Backing up Redis..."
rsync -a --quiet ~/redis_data/ $BACKUP_DIR/redis_data/

# Backup configuration
cp ~/overleaf/docker-compose.yml $BACKUP_DIR/ 2>/dev/null

# Compress backup
echo "Compressing backup..."
tar -czf $BACKUP_BASE/overleaf_backup_$TIMESTAMP.tar.gz -C $BACKUP_BASE backup_$TIMESTAMP 2>/dev/null

# Remove uncompressed backup
rm -rf $BACKUP_DIR

# Calculate backup size
BACKUP_SIZE=$(du -h $BACKUP_BASE/overleaf_backup_$TIMESTAMP.tar.gz | cut -f1)

# Keep only last N backups
cd $BACKUP_BASE
ls -t overleaf_backup_*.tar.gz 2>/dev/null | tail -n +$((KEEP_BACKUPS + 1)) | xargs -r rm

# Count remaining backups
BACKUP_COUNT=$(ls -1 overleaf_backup_*.tar.gz 2>/dev/null | wc -l)

echo "[$TIMESTAMP] Backup completed: overleaf_backup_$TIMESTAMP.tar.gz ($BACKUP_SIZE)"
echo "Total backups kept: $BACKUP_COUNT"
```

Make it executable:
```bash
chmod +x ~/backup_overleaf.sh
```

Run manually:
```bash
~/backup_overleaf.sh
```

### Schedule Automated Backups

```bash
# Edit crontab
crontab -e

# Add this line for daily backups at 2 AM
0 2 * * * /home/orion/backup_overleaf.sh >> /home/orion/overleaf_backup.log 2>&1

# Or for weekly backups every Sunday at 3 AM
0 3 * * 0 /home/orion/backup_overleaf.sh >> /home/orion/overleaf_backup.log 2>&1
```

### Restore from Backup

```bash
# Stop containers
cd ~/overleaf
docker-compose down

# Extract backup (replace TIMESTAMP with your backup date)
cd ~
tar -xzf overleaf_backup_TIMESTAMP.tar.gz

# Remove old data
rm -rf ~/sharelatex_data ~/mongo_data ~/redis_data

# Restore from backup
mv backup_TIMESTAMP/sharelatex_data ~/
mv backup_TIMESTAMP/mongo_data ~/
mv backup_TIMESTAMP/redis_data ~/

# Restore docker-compose.yml if needed
cp backup_TIMESTAMP/docker-compose.yml ~/overleaf/

# Start containers
cd ~/overleaf
docker-compose up -d

# Wait for services to start
sleep 30

echo "Restore completed. Check http://your-server:8000"
```

### Off-site Backup Options

**To External Drive:**
```bash
# Mount external drive
sudo mount /dev/sdb1 /mnt/backup

# Copy backups
cp ~/overleaf_backups/overleaf_backup_*.tar.gz /mnt/backup/
```

**To Remote Server (rsync):**
```bash
# One-time sync
rsync -avz ~/overleaf_backups/ user@remote-server:/backups/overleaf/

# Add to backup script for automatic sync
```

**To Cloud Storage (using rclone):**
```bash
# Configure rclone first
rclone config

# Sync backups
rclone sync ~/overleaf_backups/ remote:overleaf-backups/
```

---

## Logs & Debugging

### Log Locations

Inside the `sharelatex` container:

```
/var/log/overleaf/web.log              # Main web application
/var/log/overleaf/clsi.log             # Compilation service
/var/log/overleaf/document-updater.log # Document sync
/var/log/overleaf/real-time.log        # WebSocket/real-time
/var/log/overleaf/filestore.log        # File storage
/var/log/overleaf/docstore.log         # Document storage
/var/log/overleaf/chat.log             # Chat feature
/var/log/overleaf/notifications.log    # Notifications
/var/log/nginx/access.log              # Nginx access
/var/log/nginx/error.log               # Nginx errors
```

### View Logs

**From host (easiest):**
```bash
# All logs from sharelatex container
docker logs sharelatex

# Follow logs in real-time
docker logs -f sharelatex

# Last 100 lines
docker logs --tail 100 sharelatex

# Search for errors
docker logs sharelatex 2>&1 | grep -i error
```

**Inside container:**
```bash
# Enter container
docker exec -it sharelatex bash

# View specific log
tail -f /var/log/overleaf/web.log

# View last 100 lines
tail -100 /var/log/overleaf/clsi.log

# Search for errors
grep -i error /var/log/overleaf/*.log

# View all recent errors
tail -50 /var/log/overleaf/*.log | grep -i error
```

### MongoDB Logs

```bash
docker logs mongo
docker logs --tail 100 mongo
```

### Redis Logs

```bash
docker logs redis
docker logs --tail 100 redis
```

### Common Log Patterns to Look For

**Compilation Issues:**
```bash
docker exec sharelatex grep -i "compile\|clsi" /var/log/overleaf/web.log | tail -20
```

**Connection Issues:**
```bash
docker exec sharelatex grep -i "connection\|connect" /var/log/overleaf/*.log | tail -20
```

**Timeout Issues:**
```bash
docker exec sharelatex grep -i "timeout" /var/log/overleaf/*.log | tail -20
```

**MongoDB Issues:**
```bash
docker exec sharelatex grep -i "mongo" /var/log/overleaf/web.log | tail -20
```

### Debugging Compilation Problems

**Check if compiler is installed:**
```bash
docker exec sharelatex pdflatex --version
```

**Check CLSI service status:**
```bash
docker exec sharelatex curl http://localhost:3013/status
```

**Test manual compilation:**
```bash
# Enter container
docker exec -it sharelatex bash

# Go to a compile directory
cd /var/lib/overleaf/data/compiles/<project-id>

# Try compiling manually
pdflatex main.tex
```

**Check compile permissions:**
```bash
docker exec sharelatex ls -la /var/lib/overleaf/data/compiles/
```

---

## Maintenance

### Update Overleaf

```bash
# Backup first!
~/backup_overleaf.sh

# Pull latest image
cd ~/overleaf
docker-compose pull

# Restart with new image
docker-compose down
docker-compose up -d

# Check logs for issues
docker logs -f sharelatex
```

### Clean Up Old Compile Cache

```bash
# Enter container
docker exec -it sharelatex bash

# Clean old compile files (older than 7 days)
find /var/lib/overleaf/data/compiles -type d -mtime +7 -exec rm -rf {} +

# Check disk usage before
du -sh /var/lib/overleaf/data/compiles

# Clean and check after
find /var/lib/overleaf/data/compiles -type d -mtime +7 -exec rm -rf {} +
du -sh /var/lib/overleaf/data/compiles
```

### Check Disk Usage

```bash
# Overall usage
df -h

# Overleaf data
du -sh ~/sharelatex_data
du -sh ~/mongo_data
du -sh ~/redis_data

# Breakdown by subdirectory
du -h ~/sharelatex_data/* | sort -rh | head -10
```

### Restart Services

```bash
# Restart all
cd ~/overleaf
docker-compose restart

# Restart specific service
docker-compose restart sharelatex
docker-compose restart mongo
docker-compose restart redis

# Stop all
docker-compose down

# Start all
docker-compose up -d
```

### Check Service Health

```bash
# Check running containers
docker-compose ps

# Check MongoDB health
docker exec mongo mongosh --eval "db.stats()"

# Check Redis health
docker exec redis redis-cli ping

# Check Overleaf health
curl http://localhost:8000/status
```

### Reset Everything (Nuclear Option)

⚠️ **WARNING: This deletes ALL data!**

```bash
# Backup first if needed
~/backup_overleaf.sh

# Stop and remove containers
cd ~/overleaf
docker-compose down -v

# Remove all data
rm -rf ~/sharelatex_data ~/mongo_data ~/redis_data

# Start fresh
docker-compose up -d
```

---

## Quick Reference Commands

### Daily Operations
```bash
# View logs
docker logs -f sharelatex

# Restart Overleaf
docker-compose restart sharelatex

# Run backup
~/backup_overleaf.sh

# Check disk space
df -h
du -sh ~/sharelatex_data
```

### Troubleshooting
```bash
# Enter container
docker exec -it sharelatex bash

# Check logs
docker logs sharelatex | grep -i error

# Check CLSI
docker exec sharelatex curl http://localhost:3013/status

# Test compiler
docker exec sharelatex pdflatex --version

# Check service status
docker-compose ps
```

### Container Management
```bash
# Restart everything
cd ~/overleaf && docker-compose restart

# Stop everything
cd ~/overleaf && docker-compose down

# Start everything
cd ~/overleaf && docker-compose up -d

# View all logs
docker-compose logs -f
```

---

## Support & Resources

- **Official Documentation:** https://github.com/overleaf/overleaf/wiki
- **Community Edition Issues:** https://github.com/overleaf/overleaf/issues
- **Docker Hub:** https://hub.docker.com/r/sharelatex/sharelatex

### Getting Help

When asking for help, provide:
1. Docker logs: `docker logs sharelatex --tail 100`
2. Your docker-compose.yml (remove sensitive info)
3. Error messages from the Overleaf interface
4. Browser console errors (F12 → Console)

---

**Last Updated:** 2025-10-16