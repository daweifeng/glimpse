
type VideoProps = {
    id: string,
    name: string,
}

export default function Video({ id, name }: VideoProps) {

    return (
        <div>
            <div>{ name }</div>
            <video id={id} muted autoPlay playsInline></video>
        </div>
    )
}
