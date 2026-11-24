package com.apocalypse.game;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.graphics.Color;
import android.opengl.GLES30;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class NativeGameActivity extends Activity {
    private GameSurface surface;

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setStatusBarColor(Color.BLACK);
        getWindow().setNavigationBarColor(Color.BLACK);
        FrameLayout frame = new FrameLayout(this);
        surface = new GameSurface();
        frame.addView(surface, new FrameLayout.LayoutParams(-1, -1));
        TextView overlay = new TextView(this);
        overlay.setText("APOCALYPSE • MOTOR NATIVO C++ / OPENGL ES 3.0\nTela de validação do renderizador");
        overlay.setTextColor(Color.WHITE);
        overlay.setTextSize(14);
        overlay.setGravity(Gravity.TOP | Gravity.CENTER_HORIZONTAL);
        overlay.setPadding(12, 18, 12, 12);
        frame.addView(overlay);
        setContentView(frame);
    }

    @Override protected void onResume() { super.onResume(); if (surface != null) surface.onResume(); }
    @Override protected void onPause() { if (surface != null) surface.onPause(); super.onPause(); }

    private static final class GameSurface extends GLSurfaceView {
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

        @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) { nativeInit(); }
        @Override public void onSurfaceChanged(GL10 gl, int width, int height) { nativeResize(width, height); }
        @Override public void onDrawFrame(GL10 gl) { nativeRender(); }
    }
}