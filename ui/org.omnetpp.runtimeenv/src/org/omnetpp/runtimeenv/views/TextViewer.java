package org.omnetpp.runtimeenv.views;

import org.eclipse.jface.resource.JFaceResources;
import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyledTextContent;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.MouseMoveListener;
import org.eclipse.swt.events.MouseWheelListener;
import org.eclipse.swt.events.PaintEvent;
import org.eclipse.swt.events.PaintListener;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.Canvas;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.omnetpp.common.color.ColorFactory;


/**
 * For efficient viewing of a large amount of text. About 10x faster than 
 * StyledText, and has constant memory consumption (StyleTextRenderer consumes 
 * minimum 8 bytes per line, see lineWidth[] and lineHeight[] arrays).
 * 
 * @author Andras
 */
public class TextViewer extends Canvas {
    protected StyledTextContent content = null;
    protected Font font;
    protected Color backgroundColor = ColorFactory.WHITE;
    protected Color foregroundColor = ColorFactory.BLACK;
    protected int leftMargin = 0;
    protected int lineHeight; // measured from font
    protected int topLineIndex;
    protected boolean caretState;
    protected int caretLineIndex, caretColumn;
    protected int selectionStartLineIndex, selectionStartColumn;
    protected int selectionEndLineIndex, selectionEndColumn;

    // all-in-one listener
    protected class TextViewerListener implements PaintListener, KeyListener, MouseListener, MouseMoveListener, MouseWheelListener {
        @Override
        public void paintControl(PaintEvent e) {
            handlePaintEvent(e);
        }

        @Override
        public void keyPressed(KeyEvent e) {
            caretState = true;  //XXX also restart blink timer
            switch (e.keyCode) {
                case SWT.ARROW_UP:
                    caretLineIndex = Math.max(0, caretLineIndex-1);
                    redraw();
                    break;
                case SWT.ARROW_DOWN:
                    caretLineIndex = Math.max(0, Math.min(content.getLineCount()-1, caretLineIndex+1));
                    redraw();
                    break;
                case SWT.ARROW_LEFT:
                    if (caretColumn > 0) 
                        caretColumn--;
                    else if (caretLineIndex > 0) {
                        caretLineIndex--;
                        caretColumn = content.getLine(caretLineIndex).length();
                    }
                    redraw();
                    break;
                case SWT.ARROW_RIGHT:
                    if (caretColumn < content.getLine(caretLineIndex).length())
                        caretColumn++;
                    else if (caretLineIndex < content.getLineCount()-1) {
                        caretLineIndex++;
                        caretColumn = 0;
                    }
                    redraw();
                    break;
                case SWT.PAGE_DOWN:
                    int numVisibleLines = (getSize().y + lineHeight - 1) / lineHeight;
                    caretLineIndex += Math.max(1, numVisibleLines-2);
                    if (caretLineIndex >= content.getLineCount())
                        caretLineIndex = content.getLineCount()-1;
                    redraw();
                    break;
                case SWT.PAGE_UP:
                    int numVisibleLines2 = (getSize().y + lineHeight - 1) / lineHeight;
                    caretLineIndex -= Math.max(1, numVisibleLines2-2);
                    if (caretLineIndex < 0)
                        caretLineIndex = 0;
                    redraw();
                    break;
                case SWT.HOME:
                    caretColumn = 0;
                    redraw();
                    break;
                case SWT.END:
                    caretColumn = content.getLine(caretLineIndex).length();
                    redraw();
                    break;
            }
        }

        @Override
        public void keyReleased(KeyEvent e) {
            // TODO Auto-generated method stub
        }

        @Override
        public void mouseDoubleClick(MouseEvent e) {
            // TODO Auto-generated method stub
        }

        @Override
        public void mouseDown(MouseEvent e) {
            // TODO Auto-generated method stub
        }

        @Override
        public void mouseUp(MouseEvent e) {
            // TODO Auto-generated method stub
        }

        @Override
        public void mouseMove(MouseEvent e) {
            // TODO Auto-generated method stub
        }

        @Override
        public void mouseScrolled(MouseEvent e) {
            // TODO Auto-generated method stub
        }
    }
    
    public TextViewer(Composite parent, int style) {
        super(parent, style | SWT.H_SCROLL | SWT.V_SCROLL);
        
        setFont(JFaceResources.getTextFont());
        
        addListeners();
        
        // cursor blinking
        //XXX disable if not focused
        Display.getCurrent().timerExec(500, new Runnable() {
            @Override
            public void run() {
                if (!isDisposed()) {
                    caretState = !caretState;
                    redraw();
                    Display.getCurrent().timerExec(500, this);
                }
            }
        });
    }

    protected void addListeners() {
        TextViewerListener listener = new TextViewerListener();
        addPaintListener(listener);
        addKeyListener(listener);
        addMouseListener(listener);
        addMouseMoveListener(listener);
        addMouseWheelListener(listener);
    }

    public void setContent(StyledTextContent content) {
        this.content = content;
        redraw();
    }

    public StyledTextContent getContent() {
        return content;
    }
    
    public void setFont(Font font) {
        this.font = font;
        //int height = font.getFontData()[0].getHeight(); //XXX this is in points not pixels!
        lineHeight = 16; //FIXME measure or calculate from font size in points
        redraw();
    }

    public Font getFont() {
        return font;
    }

    public void setBackgroundColor(Color backgroundColor) {
        this.backgroundColor = backgroundColor;
        redraw();
    }

    public Color getBackgroundColor() {
        return backgroundColor;
    }

    public void setForegroundColor(Color foregroundColor) {
        this.foregroundColor = foregroundColor;
        redraw();
    }

    public Color getForegroundColor() {
        return foregroundColor;
    }

    public void setTopLineIndex(int topLineIndex) {
        this.topLineIndex = topLineIndex;
        redraw();
    }

    public int getTopLineIndex() {
        return topLineIndex;
    }

    public void setCaretPosition(int lineIndex, int column) {
        caretLineIndex = lineIndex;
        caretColumn = column;
    }

    public int getCaretLineIndex() {
        return caretLineIndex;
    }

    public int getCaretColumn() {
        return caretColumn;
    }

    public void setSelectionStart(int lineIndex, int column) {
        selectionStartLineIndex = lineIndex;
        selectionStartColumn = column;
        redraw();
    }

    public int getSelectionStartLineIndex() {
        return selectionStartLineIndex;
    }

    public int getSelectionStartColumn() {
        return selectionStartColumn;
    }

    public void setSelectionEnd(int lineIndex, int column) {
        selectionEndLineIndex = lineIndex;
        selectionEndColumn = column;
        redraw();
    }

    public int getSelectionEndLineIndex() {
        return selectionEndLineIndex;
    }

    public int getSelectionEndColumn() {
        return selectionEndColumn;
    }

    public int getLineHeight() {
        return lineHeight;
    }

    protected void handlePaintEvent(PaintEvent e) {
        GC gc = e.gc;
        gc.setBackground(backgroundColor);
        gc.setForeground(foregroundColor);
        gc.setFont(font);
        
        Point size = getSize();
        gc.fillRectangle(0, 0, size.x, size.y);
        
        int lineIndex = topLineIndex;
        int numLines = content.getLineCount();
        int numVisibleLines = (size.y + lineHeight - 1) / lineHeight;

        // adjust lineIndex
        if (lineIndex > numLines-numVisibleLines)
            lineIndex = numLines-numVisibleLines;
        if (lineIndex < 0)  
            lineIndex = 0;

        // draw the lines
        for (int y = 0; y < size.y && lineIndex < numLines; y += lineHeight) {
            drawLine(gc, lineIndex++, y);
        }
    }

    protected void drawLine(GC gc, int lineIndex, int y) {
        String line = content.getLine(lineIndex);
        gc.drawString(line, leftMargin, y);
        
        if (lineIndex == caretLineIndex && caretState) {
            // draw caret
            String linePrefix = caretColumn >= line.length() ? line : line.substring(0, caretColumn);
            int caretX = leftMargin + gc.textExtent(linePrefix).x;
            gc.drawRectangle(caretX, y, 1, lineHeight-1);
        }
    }

}
