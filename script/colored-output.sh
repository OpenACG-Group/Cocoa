function print_err() {
    echo -e "\033[31;1m==>\033[0m \033[1m$@\033[0m"
}

function print_info() {
    echo -e "\033[32;1m==>\033[0m $@"
}

function print_warn() {
    echo -e "\033[33;1m==>\033[0m \033[1m$@\033[0m"
}