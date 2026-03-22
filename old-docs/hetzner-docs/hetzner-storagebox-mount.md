# Hetzner Storage Box — Mount on Ubuntu (CIFS)

## Voraussetzungen (Hetzner Console)

Für den Sub-Account folgendes aktivieren:
- **SMB/CIFS** — für den Mount
- **Externe Erreichbarkeit** — falls der Server nicht bei Hetzner steht

---

## 1. Pakete installieren

```bash
sudo apt update && sudo apt install cifs-utils -y
```

---

## 2. Credentials Datei anlegen

```bash
sudo nano /etc/storagebox-credentials.txt
```

Inhalt:
```
username=u64.....
password=DEINPASSWORT
```

Datei sichern:
```bash
sudo chmod 600 /etc/storagebox-credentials.txt
```

---

## 3. Mount-Punkt anlegen

```bash
sudo mkdir -p /mnt/storagebox
```

---

## 4. Manuell testen (vor fstab!)

```bash
sudo mount.cifs //u...-sub2.your-storagebox.de/u...-sub2 /mnt/storagebox \
  -o seal,credentials=/etc/storagebox-credentials.txt,iocharset=utf8,rw
```

Prüfen ob es funktioniert:
```bash
df -h | grep storagebox
ls /mnt/storagebox
```

Wieder unmounten:
```bash
sudo umount /mnt/storagebox
```

---

## 5. Permanent via /etc/fstab

```bash
sudo nano /etc/fstab
```

Diese Zeile hinzufügen:
```
//u...-sub2.your-storagebox.de/u....-sub2  /mnt/storagebox  cifs  iocharset=utf8,rw,seal,credentials=/etc/storagebox-credentials.txt,uid=1000,gid=1000,file_mode=0660,dir_mode=0770,_netdev  0  0
```

> **Hinweis:** `uid` und `gid` mit dem eigenen User anpassen. Prüfen mit:
> ```bash
> id
> ```

fstab anwenden:
```bash
sudo mount -a
```

Prüfen:
```bash
df -h | grep storagebox
cd /mnt/storagebox
```

---

## Optionen erklärt

| Option | Bedeutung |
|---|---|
| `seal` | Verschlüsselte Verbindung (empfohlen, Ubuntu 18.04+) |
| `credentials=` | Pfad zur Credentials-Datei |
| `iocharset=utf8` | Zeichensatz für Dateinamen |
| `rw` | Read/Write Zugriff |
| `uid=1000` | Linux User bekommt Zugriff (eigene UID eintragen) |
| `gid=1000` | Linux Gruppe bekommt Zugriff (eigene GID eintragen) |
| `file_mode=0660` | Dateiberechtigungen |
| `dir_mode=0770` | Verzeichnisberechtigungen |
| `_netdev` | Warten auf Netzwerk vor dem Mount beim Boot |

---

## Troubleshooting

**Permission denied beim cd:**
```bash
# uid/gid in fstab prüfen und anpassen
id
# Dann neu mounten
sudo umount /mnt/storagebox && sudo mount -a
```

**Port 445 geblockt (z.B. durch ISP):**
- Einige ISPs blockieren SMB/CIFS auf Port 445
- Alternative: SSHFS verwenden (SSH in Hetzner Console aktivieren)

**Große Dateien (>4GB) machen Probleme:**
```
# cache=none zur fstab Option hinzufügen
...,_netdev,cache=none  0  0
```
