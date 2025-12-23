//-----------------------------------------------------------------------------
// Copyright (c) 2025 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

unit uMain;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, StdCtrls,
  MaskEdit, ExtDlgs
  , Math;

type

  { TForm1 }

  { TfmBitmapFont }

  TfmBitmapFont = class(TForm)
    btnFont: TButton;
    btnBackGroundColor: TButton;
    btnSaveAs: TButton;
    chkDrawLines: TCheckBox;
    chkAntiAlasing: TCheckBox;
    ColorDialog1: TColorDialog;
    edHeight: TEdit;
    edWidth: TEdit;
    FontDialog1: TFontDialog;
    Label1: TLabel;
    Label2: TLabel;
    PaintBox1: TPaintBox;
    Panel1: TPanel;
    SavePictureDialog1: TSavePictureDialog;
    ScrollBox1: TScrollBox;
    procedure btnBackGroundColorClick(Sender: TObject);
    procedure btnFontClick(Sender: TObject);
    procedure btnSaveAsClick(Sender: TObject);
    procedure chkDrawLinesChange(Sender: TObject);
    procedure edHeightEditingDone(Sender: TObject);
    procedure edWidthEditingDone(Sender: TObject);
    procedure FormCreate(Sender: TObject);
    procedure PaintBox1Paint(Sender: TObject);
  private

    backGroundColor : TColor;
    charW, charH : integer;
    procedure onFontChanged();
    procedure onSizeChanged();
    procedure PaintFont(myCanvas:TCanvas);
  public

  end;

var
  fmBitmapFont: TfmBitmapFont;


implementation

{$R *.lfm}

{ TForm1 }


procedure TfmBitmapFont.onSizeChanged();
 begin

  PaintBox1.width  := charW * 10;
  PaintBox1.height := charH * 10;

  Invalidate;
end;

procedure TfmBitmapFont.edHeightEditingDone(Sender: TObject);
begin
  charH := StrToIntDef(edHeight.text, 16);
  onSizeChanged();

end;

procedure TfmBitmapFont.btnFontClick(Sender: TObject);
begin
  FontDialog1.font := PaintBox1.font;
  if (FontDialog1.Execute) then
  begin
    PaintBox1.font := FontDialog1.font;
    onFontChanged();
    Invalidate;
  end;
end;

procedure TfmBitmapFont.btnBackGroundColorClick(Sender: TObject);
begin
  ColorDialog1.Color:=backGroundColor;
  if (ColorDialog1.Execute) then begin
     backGroundColor := ColorDialog1.Color;
     btnBackGroundColor.Color:= backGroundColor;
  end;

end;

procedure TfmBitmapFont.btnSaveAsClick(Sender: TObject);
 var bmp : TBitmap;
 var savDrawLines: Boolean;
begin
  if not SavePictureDialog1.Execute then
     exit;

  savDrawLines := chkDrawLines.Checked;
  chkDrawLines.Checked := false;
  bmp:=TBitmap.Create;
  try
     bmp.SetSize(PaintBox1.Width,PaintBox1.height);
     bmp.Canvas.Font := PaintBox1.Font;
     bmp.Canvas.Font.PixelsPerInch := Screen.PixelsPerInch;
     PaintFont(bmp.Canvas);
     bmp.SaveToFile(SavePictureDialog1.FileName);
     chkDrawLines.Checked :=  savDrawLines;
  finally
    bmp.free;
  end;

end;

procedure TfmBitmapFont.chkDrawLinesChange(Sender: TObject);
begin
  Invalidate;
end;

procedure TfmBitmapFont.edWidthEditingDone(Sender: TObject);
begin
 charW := StrToIntDef(edWidth.text, 16);
 onSizeChanged();

end;

procedure TfmBitmapFont.FormCreate(Sender: TObject);
begin
  PaintBox1.width  := 512;
  PaintBox1.height := 512;
  onFontChanged();
  backGroundColor := clFuchsia;
  btnBackGroundColor.Color:= backGroundColor;
end;

procedure TfmBitmapFont.onFontChanged();
 var i, w, h:Integer;
begin
  charW := 0;
  charH := 0;
  for i:=32 to 126 do
  begin
    w := PaintBox1.Canvas.GetTextWidth(chr(i));
    if ( w > charW ) then
       charW := w;
    h := PaintBox1.Canvas.GetTextHeight(chr(i));
    if ( h > charH ) then
       charH := h;
  end;

  PaintBox1.width  := charW * 10;
  PaintBox1.height := charH * 10;


  edWidth.text  := inttostr(charW);
  edHeight.text := inttostr(charH);

  btnFont.caption := PaintBox1.Font.Name + ' '+ IntToStr(PaintBox1.Font.Size);
end;

procedure TfmBitmapFont.PaintBox1Paint(Sender: TObject);

begin
  PaintFont(PaintBox1.Canvas);
end;

procedure TfmBitmapFont.PaintFont(myCanvas:TCanvas);
 var i, x, y, w, h, addX:integer;

begin
  if (chkAntiAlasing.Checked) then
  begin
    myCanvas.font.Quality:= fqAntialiased;
    myCanvas.AntialiasingMode := amOn;

  end
  else
  begin
    myCanvas.font.Quality:= fqNonAntialiased;
    myCanvas.AntialiasingMode := amOff;

  end;

  myCanvas.Brush.Color:=backGroundColor;
  myCanvas.Pen.Color:=backGroundColor;
  myCanvas.Rectangle(0,0,PaintBox1.width ,PaintBox1.Height);
  x := 0;
  y := 0;
  if chkDrawLines.Checked then
  begin
    myCanvas.Pen.Color:=TColor($FFFFFF);
  end;

  for i:=32 to 126 do
  begin

    //x center
    w := myCanvas.GetTextWidth(chr(i));
    addX := Max(0, (charW - w) div 2);
    // addX := trunc((charW - w) / 2);
    myCanvas.TextOut(x + addX ,y,chr(i));
    if chkDrawLines.Checked then
    begin
      myCanvas.Line(x,y,x,y+charH );
      myCanvas.Line(x,y,x+charW ,y);
    end;
    x := x + charW ;

    //hackfest width
    if ( x + charW  > PaintBox1.width) then begin
      x:=0;
      y:=y+charH;
    end;
  end;
end;


end.

