# Manual: How to Install and Run Docker in WSL (Ubuntu 26.04)

This manual guides you through installing Docker natively inside an Ubuntu 26.04 WSL (Windows Subsystem for Linux) environment and launching a pristine, isolated non-root user sandbox for testing installation and build scripts.

---

## Part 1: Installing Docker Engine Natively in WSL

Because WSL 2 runs on a real Linux kernel, you can install the lightweight Docker Engine directly inside your distro via `apt` without needing Windows-based desktop applications.

### Step 1: Update Packages & Install Prerequisites

Open your Ubuntu WSL terminal and run:

```bash
sudo apt update
sudo apt install -y ca-certificates curl gnupg

```

### Step 2: Add Docker's Official GPG Key and Repository

```bash
sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
sudo chmod a+r /etc/apt/keyrings/docker.gpg

echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

```

### Step 3: Install Docker Engine Components

```bash
sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

```

### Step 4: Manage Background Services

WSL does not always boot system services automatically. You must start the Docker daemon manually when beginning a work session:

```bash
sudo service docker start

```

---

## Part 2: Configuring Non-Sudo Permissions

By default, the Docker Unix socket is owned by `root`. To avoid typing `sudo` before every single `docker` command, assign your WSL user to the docker security group.

### Step 1: Add User to the Group

```bash
sudo usermod -aG docker $USER

```

### Step 2: Refresh Group Assignments

Instead of restarting your whole WSL instance, force your current terminal session to reload your group memberships immediately:

```bash
newgrp docker

```

---

## Part 3: Launching the Sandbox Environment

Official Ubuntu Docker images are stripped down to minimize size. Out of the box, they **do not include the `sudo` package**, and the default `ubuntu` user has no password assigned (meaning commands like `su` or `sudo` will fail with authentication errors).

To bypass this roadblock, use the automated command below. It spins up the container as root momentarily to install `sudo`, grants the standard `ubuntu` user passwordless administrative privileges, maps your current directory, and drops you into a non-root environment.

### The Comprehensive Test Command

Navigate (`cd`) to your project folder inside WSL and execute:

```bash
docker run --rm -it \
  -v "$(pwd)":/workspace -w /workspace \
  ubuntu:26.04 \
  bash -c "apt-get update && apt-get install -y sudo && echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && exec su - ubuntu -c 'cd /workspace && exec bash'"

```

### Command Parameter Breakdown

* `--rm`: Automatically deletes the container and its internal file modifications immediately upon exit, keeping your machine completely free of leftover junk.
* `-it`: Keeps the terminal window interactive, connecting your keyboard input directly to the shell inside the container.
* `-v "$(pwd)":/workspace`: Creates a volume mount portal linking your host WSL directory (`$(pwd)`) to the `/workspace` folder inside the container.
* `-w /workspace`: Sets the default container working directory, executing an automatic `cd /workspace` so you start exactly where your files reside.
* `bash -c "..."`: Safely handles initialization under the root user profile before dropping terminal permissions down to the target test account.

---

## Part 4: Testing Inside the Sandbox

Once execution finishes, you may see some warning message like:

```text
bash: cannot set terminal process group (-1): Inappropriate ioctl for device
bash: no job control in this shell
```

You can ignore the warnings, and your prompt will change to:

```text
ubuntu@<container_id>:/workspace$

```

You are now a normal, non-root user sitting inside a clean environment, ready to evaluate permission limits and execution safety.

### Verification Checkmarks

1. **Identity Check:** Type `whoami`. The output will read `ubuntu`.
2. **Elevated Execution:** Run `sudo apt update`. The package lists will update cleanly without prompting you for a password.
3. **File Synchronization:** Any changes made to code files or dependencies inside `/workspace` will reflect instantly on your host hard drive.

To destroy the sandbox and return to your primary terminal environment, simply type:

```bash
exit

```
