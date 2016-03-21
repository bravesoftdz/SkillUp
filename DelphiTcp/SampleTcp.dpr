program SampleTcp;

uses
  System.StartUpCopy,
  FMX.Forms,
  Main in 'Main.pas' {TcpForm};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(TTcpForm, TcpForm);
  Application.Run;
end.
