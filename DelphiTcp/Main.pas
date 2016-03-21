unit Main;

interface

uses
  System.SysUtils, System.Types, System.UITypes, System.Classes, System.Variants,
  FMX.Types, FMX.Controls, FMX.Forms, FMX.Graphics, FMX.Dialogs,
  IdBaseComponent, IdComponent, IdTCPConnection, IdTCPClient, FMX.StdCtrls,
  FMX.ScrollBox, FMX.Memo, FMX.Controls.Presentation, FMX.Edit, FMX.Objects;

type
  TTcpForm = class(TForm)
    Edit1: TEdit;
    Memo1: TMemo;
    IdTCPClient1: TIdTCPClient;
    Timer1: TTimer;
    Rectangle1: TRectangle;
    procedure Edit1Change(Sender: TObject);
    procedure Timer1Timer(Sender: TObject);
  private
    { private êÈåæ }
  public
    { public êÈåæ }
  end;

var
  TcpForm: TTcpForm;

implementation

{$R *.fmx}


procedure TTcpForm.Edit1Change(Sender: TObject);
begin
     if Edit1.Text = Edit1.Text.Empty Then Exit;
     if not IdTCPClient1.Connected then
                         IdTCPClient1.Connect;
     IdTcpClient1.IOHandler.WriteLn( Edit1.Text );


end;

procedure TTcpForm.Timer1Timer(Sender: TObject);
begin
     if IdTCPClient1.Connected then
     begin
        if IdTcpClient1.IOHandler.InputBufferIsEmpty then exit;
        Memo1.Lines.Add( IdTcpClient1.IOHandler.ReadLn());
     end;
end;

end.
