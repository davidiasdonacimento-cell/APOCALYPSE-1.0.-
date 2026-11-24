package com.apocalypse.game;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.view.animation.AlphaAnimation;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class Verity3DActivity extends Activity {
    private GLSurfaceView surface;
    private View intro;
    private final Handler handler = new Handler(Looper.getMainLooper());
    private final Runnable finishIntro = () -> {
        if (intro != null) {
            intro.animate().alpha(0f).setDuration(550).withEndAction(() -> {
                if (intro != null) intro.setVisibility(View.GONE);
            }).start();
        }
    };

    private int dp(float v) {
        return (int)(v * getResources().getDisplayMetrics().density + .5f);
    }

    private Button button(String text, int color) {
        Button b = new Button(this);
        b.setText(text);
        b.setTextColor(Color.WHITE);
        b.setAllCaps(false);
        b.setTextSize(14);
        b.setBackgroundColor(color);
        return b;
    }

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setStatusBarColor(Color.rgb(5, 9, 20));
        getWindow().setNavigationBarColor(Color.rgb(5, 9, 20));

        FrameLayout root = new FrameLayout(this);
        root.setBackgroundColor(Color.rgb(5, 9, 20));

        surface = new GLSurfaceView(this);
        surface.setEGLContextClientVersion(3);
        surface.setRenderer(new Renderer());
        surface.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        root.addView(surface, new FrameLayout.LayoutParams(-1, -1));

        TextView title = new TextView(this);
        title.setText("APOCALYPSE  •  LOBBY 3D");
        title.setTextColor(Color.WHITE);
        title.setTextSize(17);
        title.setGravity(Gravity.CENTER);
        title.setPadding(dp(12), dp(18), dp(12), dp(12));
        FrameLayout.LayoutParams titleParams = new FrameLayout.LayoutParams(-1, -2, Gravity.TOP);
        root.addView(title, titleParams);

        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(14), dp(12), dp(14), dp(14));
        panel.setBackgroundColor(0xDD101A31);

        TextView status = new TextView(this);
        status.setText("VERITY\nEstou aqui. Podemos conversar antes de partir.");
        status.setTextColor(Color.rgb(210, 230, 255));
        status.setTextSize(14);
        status.setGravity(Gravity.CENTER);
        panel.addView(status, new LinearLayout.LayoutParams(-1, -2));

        Button talk = button("MENSAGEM PARA VERITY", Color.rgb(82, 49, 128));
        LinearLayout.LayoutParams p1 = new LinearLayout.LayoutParams(-1, dp(48));
        p1.topMargin = dp(8);
        panel.addView(talk, p1);
        talk.setOnClickListener(v -> {
            surface.queueEvent(() -> Renderer.nativeSetAnimation(2));
            startActivity(new Intent(this, VeritySceneActivity.class));
        });

        Button wave = button("ACENAR / EXPRESSÃO", Color.rgb(35, 91, 126));
        LinearLayout.LayoutParams waveParams = new LinearLayout.LayoutParams(-1, dp(46));
        waveParams.topMargin = dp(7);
        panel.addView(wave, waveParams);
        wave.setOnClickListener(v -> surface.queueEvent(() -> Renderer.nativeSetAnimation(1)));

        Button walk = button("TESTAR ANIMAÇÃO DE CAMINHADA", Color.rgb(65, 76, 102));
        LinearLayout.LayoutParams walkParams = new LinearLayout.LayoutParams(-1, dp(46));
        walkParams.topMargin = dp(7);
        panel.addView(walk, walkParams);
        walk.setOnClickListener(v -> surface.queueEvent(() -> Renderer.nativeSetAnimation(3)));

        Button start = button("COMEÇAR PARTIDA", Color.rgb(22, 116, 91));
        LinearLayout.LayoutParams p2 = new LinearLayout.LayoutParams(-1, dp(52));
        p2.topMargin = dp(7);
        panel.addView(start, p2);
        start.setOnClickListener(v -> startActivity(new Intent(this, NativeGameActivity.class)));

        FrameLayout.LayoutParams panelParams = new FrameLayout.LayoutParams(
                -1, -2, Gravity.BOTTOM);
        panelParams.setMargins(dp(12), 0, dp(12), dp(14));
        root.addView(panel, panelParams);

        intro = new LinearLayout(this);
        ((LinearLayout) intro).setOrientation(LinearLayout.VERTICAL);
        ((LinearLayout) intro).setGravity(Gravity.CENTER);
        intro.setBackgroundColor(0xF2050914);
        TextView introTitle = new TextView(this);
        introTitle.setText("VERITY");
        introTitle.setTextColor(Color.WHITE);
        introTitle.setTextSize(30);
        introTitle.setGravity(Gravity.CENTER);
        ((LinearLayout) intro).addView(introTitle);
        TextView introLine = new TextView(this);
        introLine.setText("A jornada começa aqui.\nFale comigo antes de entrar no mundo.");
        introLine.setTextColor(Color.rgb(120, 205, 255));
        introLine.setTextSize(15);
        introLine.setGravity(Gravity.CENTER);
        introLine.setPadding(dp(22), dp(14), dp(22), dp(14));
        ((LinearLayout) intro).addView(introLine);
        Button skip = button("PULAR APRESENTAÇÃO", Color.rgb(47, 67, 96));
        ((LinearLayout) intro).addView(skip);
        skip.setOnClickListener(v -> finishIntro.run());
        root.addView(intro, new FrameLayout.LayoutParams(-1, -1));

        setContentView(root);
        handler.postDelayed(finishIntro, 2300);
    }

    @Override protected void onResume() {
        super.onResume();
        if (surface != null) { surface.onResume(); surface.queueEvent(() -> Renderer.nativeSetAnimation(0)); }
    }

    @Override protected void onPause() {
        handler.removeCallbacks(finishIntro);
        if (surface != null) surface.onPause();
        super.onPause();
    }

    @Override protected void onDestroy() {
        handler.removeCallbacks(finishIntro);
        super.onDestroy();
    }

    private static final class Renderer implements GLSurfaceView.Renderer {
        static { System.loadLibrary("apocalypse_engine"); }
        private static native void nativeInit();
        private static native void nativeResize(int width, int height);
        private static native void nativeRender();
        private static native void nativeSetAnimation(int mode);
        @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) { nativeInit(); }
        @Override public void onSurfaceChanged(GL10 gl, int width, int height) { nativeResize(width, height); }
        @Override public void onDrawFrame(GL10 gl) { nativeRender(); }
    }
}
