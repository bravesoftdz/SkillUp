program SampleUdp;

uses
  System.StartUpCopy,
  FMX.Forms,
  Main in 'Main.pas' {UdpForm};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TUdpForm, UdpForm);
  Application.Run;
end.
