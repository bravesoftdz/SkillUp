unit Main;

interface

uses
  System.SysUtils, System.Types, System.UITypes, System.Classes, System.Variants,
  FMX.Types, FMX.Controls, FMX.Forms, FMX.Graphics, FMX.Dialogs, IdUDPServer,
  IdGlobal, IdSocketHandle, IdBaseComponent, IdComponent, IdUDPBase,
  FMX.StdCtrls, FMX.Edit, FMX.Controls.Presentation, FMX.ScrollBox, FMX.Memo,
  FMX.Objects;

type
  TUdpForm = class(TForm)
    IdUDPServer1: TIdUDPServer;
    Memo1: TMemo;
    LeftButton: TButton;
    RightButton: TButton;
    Rectangle1: TRectangle;
    procedure IdUDPServer1UDPRead(AThread: TIdUDPListenerThread;
      const AData: TIdBytes; ABinding: TIdSocketHandle);
    procedure ButtonMouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Single);
  private
    { private �錾 }
  public
    { public �錾 }
  end;

var
  UdpForm: TUdpForm;

implementation

{$R *.fmx}


procedure TUdpForm.ButtonMouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Single);
begin
    if not (Sender is TButton) then exit;
      case (Sender As TButton).Tag of
            0:  IdUDPServer1.Send('192.168.0.1', 65002,'Left');
            1:  IdUDPServer1.Send('192.168.0.1', 65002,'Right');
      end;
end;

procedure TUdpForm.IdUDPServer1UDPRead(AThread: TIdUDPListenerThread;
  const AData: TIdBytes; ABinding: TIdSocketHandle);
var
 Str  : String;
begin
{
    Adata[0] : '+' or '-'
    Adata[1] : ���x���
    Adata[2] : ���x����
    Adata[3] = '.'
    Adata[4] : ���x�����_�ȉ����
    Adata[5] : ���x�����_�ȉ�����
}

    if(ABinding.PeerIP <> '192.168.0.1') then exit;          // �o�����`�F�b�N!!
    str := BytesToString(AData)+'�� '+DateTimeToStr(Now());  // ������ɕϊ����A����ǉ��{���łɎ擾���Ԃ��B
    Memo1.Lines.Add(str);                                    // MEMO�ɒǉ�
end;

end.
