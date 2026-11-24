package com.apocalypse.game;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.graphics.Color;
import android.graphics.Canvas;
import android.graphics.Paint;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class NativeGameActivity extends Activity {
    private GameSurface surface;
    private Joystick joystick;

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setStatusBarColor(Color.BLACK);
        getWindow().setNavigationBarColor(Color.BLACK);
        FrameLayout frame = new FrameLayout(this);
        surface = new GameSurface();
        frame.addView(surface, new FrameLayout.LayoutParams(-1, -1));

        TextView overlay = new TextView(this);
        overlay.setText("APOCALYPSE • EXPLORAÇÃO\nJoystick: mover personagem");
        overlay.setTextColor(Color.WHITE);
        overlay.setTextSize(13);
        overlay.setGravity(Gravity.TOP | Gravity.CENTER_HORIZONTAL);
        overlay.setPadding(12, 18, 12, 12);
        frame.addView(overlay);

        joystick = new Joystick();
        int size = (int)(156 * getResources().getDisplayMetrics().density);
        FrameLayout.LayoutParams controls = new FrameLayout.LayoutParams(size, size, Gravity.BOTTOM | Gravity.START);
        controls.setMargins(20, 0, 0, 24);
        frame.addView(joystick, controls);
        setContentView(frame);
    }

    @Override protected void onResume() { super.onResume(); if (surface != null) surface.onResume(); }
    @Override protected void onPause() { if (joystick != null) joystick.release(); if (surface != null) surface.onPause(); super.onPause(); }

    private final class Joystick extends View {
        private final Paint paint = new Paint(3);
        private float cx, cy, knobX, knobY, radius;
        private boolean active = false;
        Joystick() { super(NativeGameActivity.this); setLayerType(View.LAYER_TYPE_SOFTWARE, null); }
        @Override protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            cx=getWidth()/2f; cy=getHeight()/2f; radius=Math.min(cx,cy)*0.78f;
            paint.setColor(0x553A9DCE); canvas.drawCircle(cx,cy,radius,paint);
            paint.setColor(0xCCB8E8FF); paint.setStyle(Paint.Style.STROKE); paint.setStrokeWidth(3);
            canvas.drawCircle(cx,cy,radius,paint); paint.setStyle(Paint.Style.FILL);
            float kx=active?knobX:cx, ky=active?knobY:cy;
            paint.setColor(0xDDFFFFFF); canvas.drawCircle(kx,ky,radius*0.34f,paint);
        }
        @Override public boolean onTouchEvent(MotionEvent e) {
            float dx=e.getX()-cx, dy=e.getY()-cy;
            if(e.getAction()==MotionEvent.ACTION_DOWN || e.getAction()==MotionEvent.ACTION_MOVE){
                float len=(float)Math.sqrt(dx*dx+dy*dy), max=radius;
                if(len>max){dx=dx/len*max;dy=dy/len*max;}
                knobX=cx+dx;knobY=cy+dy;active=true;
                NativeRenderer.setMove(dx/max,-dy/max);invalidate();return true;
            }
            if(e.getAction()==MotionEvent.ACTION_UP || e.getAction()==MotionEvent.ACTION_CANCEL){release();return true;}
            return true;
        }
        void release(){active=false;NativeRenderer.setMove(0f,0f);postInvalidate();}
    }

    private final class GameSurface extends GLSurfaceView {
        GameSurface() {
            super(NativeGameActivity.this);
            setEGLContextClientVersion(3);
            setRenderer(new NativeRenderer());
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        }
    }

    private static final class NativeRenderer implements GLSurfaceView.Renderer {
        static { System.loadLibrary("apocalypse_engine"); }
        private static native void nativeInit();
        private static native void nativeResize(int width, int height);
        private static native void nativeRender();
        private static native void nativeSetMove(float x, float z);
        static void setMove(float x,float z){nativeSetMove(x,z);}
        @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) { nativeInit(); }
        @Override public void onSurfaceChanged(GL10 gl, int width, int height) { nativeResize(width, height); }
        @Override public void onDrawFrame(GL10 gl) { nativeRender(); }
    }
}
