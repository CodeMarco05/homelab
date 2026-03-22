# Topic-Based Documentation Structure

This directory contains the reorganized homelab documentation structured by topics rather than dates.

## Structure Overview

```
src/topics/
├── homeserver/
│   ├── hardware-setup.tex        # ZFS setup, hardware configuration
│   └── all.tex                   # Homeserver compilation file
├── kubernetes/
│   ├── cluster-setup.tex         # K3s installation and basic setup
│   ├── networking.tex            # Tailscale, Nginx Proxy Manager
│   ├── node-management.tex       # Taints, tolerations, scheduling
│   ├── resource-management.tex   # systemd resource limits
│   ├── ssl-management.tex        # cert-manager, Let's Encrypt
│   ├── monitoring.tex            # Prometheus, Grafana, AlertManager
│   └── all.tex                   # Kubernetes compilation file
└── all.tex                       # Master topics compilation file
```

## Content Mapping

### From Date-Based to Topic-Based:

**Juli/15.tex** → **homeserver/hardware-setup.tex**
- ZFS pool creation and management
- SSD formatting and setup
- Hardware configuration

**August/8.tex** → **kubernetes/cluster-setup.tex** + **kubernetes/networking.tex**
- K3s cluster installation
- VPN configuration with Tailscale
- Nginx Proxy Manager setup

**August/9.tex** → Integrated into **kubernetes/networking.tex**
- Nginx Ingress Controller setup

**August/13.tex** → Split into multiple focused files:
- **kubernetes/node-management.tex** - Node taints and scheduling
- **kubernetes/ssl-management.tex** - SSL setup framework
- **kubernetes/monitoring.tex** - Monitoring stack
- **kubernetes/resource-management.tex** - Resource limits

## Benefits of Topic-Based Organization

1. **Logical Grouping**: Related content is grouped together
2. **Easy Navigation**: Find content by topic, not date
3. **Better Maintenance**: Updates can be made to specific topics
4. **Scalability**: New topics can be added easily
5. **Reference-Friendly**: Easier to reference specific technical areas

## Usage

The main compilation file `main.tex` now includes `src/topics/all.tex` instead of the date-based files. The old date-based structure is preserved but commented out for reference.