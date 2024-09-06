type VideoProps = {
  id: string;
  name: string;
  isMuted: boolean;
};

export default function MainVideo({ id, name, isMuted }: VideoProps) {
  return (
    <div className="w-full h-full relative">
      <div>{name}</div>
      <video
        id={id}
        muted={isMuted}
        autoPlay
        playsInline
        className="absolute left-1/2 top-1/2 transform -translate-x-1/2 -translate-y-1/2 w-full h-auto max-h-full"
      ></video>
    </div>
  );
}
