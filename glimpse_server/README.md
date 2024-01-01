# Glimpse Server
Welcome to the Glimpse Server! This is the backend service for the Glimpse application, responsible for managing private video chat rooms. The server is built using C++ and handles all the core functionalities such as creating, joining, and managing rooms.


## Development

### Preqrequisites

We are using DevContainer for development. Make sure you have the following installed:
- [Docker](https://docs.docker.com/get-docker/)
- [Visual Studio Code](https://code.visualstudio.com/)
- [Remote - Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

### Getting Started

1. Start Visual Studio Code and open the project folder.
2. Click on the green button in the bottom left corner and select `Reopen in Container`.
3. The DevContainer will start building and you will be inside the container.
4. Open a terminal and run the following commands:
    ```bash
    cmake --preset=default
    cmake --build build
    ./build/glimpse-server
    ```
5. The server should now be running and you can connect to it using the Glimpse client.
