#!/bin/bash

# --- Configuration Variables ---
# Set these paths according to your actual project structure
PATH_TO_CSR="/path/to/your/csr_project_root"
PATH_TO_CR_COLLEG="/path/to/your/cr_colleg_project_root"
SOURCE_SCRIPT_CR_COLLEG="your_source_script.sh" # e.g., setup_env.sh or activate.sh
CONDA_ENV_NAME="your_conda_env_name" # Replace with the name of your conda environment
YOUR_COMMAND_IN_CR_COLLEG="your_actual_command_here" # The command to run after conda activation

# --- Step 1: Change directory to /pathToCSR ---
echo "--- Step 1: Changing directory to $PATH_TO_CSR ---"
cd "$PATH_TO_CSR" || { echo "Error: Could not change directory to $PATH_TO_CSR. Exiting."; exit 1; }
echo "Current directory: $(pwd)"

# --- Step 2: Run docker compose command (unendlich process starting) ---
echo "--- Step 2: Running docker compose command (this might start an infinite process) ---"
# Option A: Run in background (recommended for "infinite" processes)
# We'll use `nohup` and `&` to run it in the background and detach it from the terminal.
# The output will be redirected to docker_compose_output.log
echo "Running 'docker compose run --build dev-env \"make -C csr_rsp/ demo_host-clang\"' in the background."
nohup docker compose run --build dev-env "make -C csr_rsp/ demo_host-clang" > docker_compose_output.log 2>&1 &
DOCKER_PID=$! # Store the PID of the background process
echo "Docker compose process started with PID: $DOCKER_PID. Output redirected to docker_compose_output.log"
sleep 5 # Give the docker compose command a moment to start up

# Option B: (Alternative - if the process is NOT truly infinite and you need to wait)
# If the "unendlich process" eventually finishes, you could just run it directly.
# However, your description implies it's long-running or infinite.
# docker compose run --build dev-env "make -C csr_rsp/ demo_host-clang"

# --- Step 3: Starting another process ---
echo "--- Step 3: Starting another process ---"
# Replace this with your actual command for the "another process"
# Example: If it's a server, you might run it in the background like the docker compose.
# If it's a quick command, just run it.

# Example of a background process:
# nohup your_other_command arg1 arg2 > other_process_output.log 2>&1 &
# OTHER_PROCESS_PID=$!
# echo "Another process started with PID: $OTHER_PROCESS_PID. Output redirected to other_process_output.log"

# Example of a foreground process (if it completes quickly):
echo "Running a placeholder for 'another process'."
# your_other_command_here # Uncomment and replace with your actual command

# --- Step 4: Go to folder of cr colleg ---
echo "--- Step 4: Changing directory to $PATH_TO_CR_COLLEG ---"
cd "$PATH_TO_CR_COLLEG" || { echo "Error: Could not change directory to $PATH_TO_CR_COLLEG. Exiting."; exit 1; }
echo "Current directory: $(pwd)"

# --- Step 5: Source script ---
echo "--- Step 5: Sourcing script: $SOURCE_SCRIPT_CR_COLLEG ---"
# Using `.` or `source` to execute the script in the current shell environment
if [ -f "$SOURCE_SCRIPT_CR_COLLEG" ]; then
    . "$SOURCE_SCRIPT_CR_COLLEG" || { echo "Error: Could not source $SOURCE_SCRIPT_CR_COLLEG. Exiting."; exit 1; }
    echo "Script sourced successfully."
else
    echo "Error: Source script '$SOURCE_SCRIPT_CR_COLLEG' not found in $PATH_TO_CR_COLLEG. Exiting."
    exit 1
fi

# --- Step 6: Activate Conda environment ---
echo "--- Step 6: Activating Conda environment: $CONDA_ENV_NAME ---"
# Check if conda is available
if command -v conda &> /dev/null
then
    # Initialize conda if not already initialized in the current shell session
    # This is often needed in non-interactive shell scripts
    if ! type __conda_setup > /dev/null 2>&1; then
        echo "Initializing Conda..."
        # Replace with your actual conda init path if different, or source your .bashrc/.zshrc
        # For example, if you know where your conda.sh is:
        # source /path/to/your/miniconda3/etc/profile.d/conda.sh
        # A more robust way might be to source your shell's rc file:
        # source ~/.bashrc # or ~/.zshrc
        # For this script to be portable, it's better to ensure conda is already in PATH
        # or that your .bashrc/.zshrc setup is handled outside this script.
        # For simplicity, we'll assume `conda` is in PATH or this script is run after interactive shell setup.

        # A common way to activate conda in scripts:
        eval "$(conda shell.bash hook)"
    fi
    conda activate "$CONDA_ENV_NAME" || { echo "Error: Could not activate Conda environment '$CONDA_ENV_NAME'. Exiting."; exit 1; }
    echo "Conda environment '$CONDA_ENV_NAME' activated."
else
    echo "Error: Conda command not found. Please ensure Conda is installed and in your PATH. Exiting."
    exit 1
fi

# --- Step 7: Run final command ---
echo "--- Step 7: Running final command in the activated environment ---"
# Replace this with the actual command you want to run
echo "Executing: $YOUR_COMMAND_IN_CR_COLLEG"
"$YOUR_COMMAND_IN_CR_COLLEG" || { echo "Error: Final command failed. Exiting."; exit 1; }
echo "Final command completed successfully."

# --- Cleanup/Final Notes ---
echo "Script finished."
echo "Remember to manually manage the background Docker process (PID: $DOCKER_PID) if it's an infinite process."
echo "You can kill it later using: kill $DOCKER_PID"
# If you started another process in the background, you'd mention its PID too.
