type VideoProps = {
  id: string;
  name: string;
  isMuted: boolean;
};

export default function MiniVideo({ id, name, isMuted }: VideoProps) {
  return (
    <div className="absolute w-24 h-24 right-1 top-1">
      <div>{name}</div>
      <video
        id={id}
        muted={isMuted}
        autoPlay
        playsInline
        className="absolute left-1/2 top-1/2 transform -translate-x-1/2 -translate-y-1/2 z-10"
      ></video>
    </div>
  );
}
