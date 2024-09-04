type VideoProps = {
  id: string;
  name: string;
  isMuted: boolean;
};

export default function Video({ id, name, isMuted }: VideoProps) {
  return (
    <div>
      <div>{name}</div>
      <video id={id} muted={isMuted} autoPlay playsInline></video>
    </div>
  );
}
