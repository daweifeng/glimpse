
export const serverUrl = `${process.env.NEXT_PUBLIC_SERVER_SECURE == "true" ? "https" : "http"}://${process.env.NEXT_PUBLIC_SERVER_HOST}:${process.env.NEXT_PUBLIC_SERVER_PORT}`;

export const createRoom = async (username: string, userId: string) => {
    const response = await fetch(`${serverUrl}/room`, {
        method: "POST",
        body: JSON.stringify({ username, userId }),
        headers: {
            "Content-Type": "application/json",
        },
    });
    if (!response.ok) {
        throw new Error("Failed to create room");
    }
    return await response.json();
}
