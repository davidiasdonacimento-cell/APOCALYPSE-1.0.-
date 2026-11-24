package com.apocalypse.game;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.animation.ValueAnimator;

/**
 * Lobby de pré-partida da VERITY.
 * A cena é uma apresentação 2D leve; não substitui um modelo 3D do motor.
 */
public final class VerityLobbyActivity extends Activity {
    private FrameLayout root;
    private VerityPortrait portrait;
    private LinearLayout actions;
    private TextView status;
    private View intro;
    private ValueAnimator entrance;
    private boolean lobbyReady = false;

    private int dp(float v) { return (int)(v * getResources().getDisplayMetrics().density + .5f); }

    private TextView label(String value, int size, int color, boolean bold) {
        TextView t = new TextView(this);
        t.setText(value);
        t.setTextSize(size);
        t.setTextColor(color);
        if (bold) t.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        return t;
    }

    private Button makeButton(String value, int color) {
        Button b = new Button(this);
        b.setText(value);
        b.setTextColor(Color.WHITE);
        b.setTextSize(15);
        b.setAllCaps(false);
        b.setBackgroundColor(color);
        return b;
    }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setStatusBarColor(Color.rgb(7, 10, 22));
        getWindow().setNavigationBarColor(Color.rgb(7, 10, 22));

        root = new FrameLayout(this);
        root.setBackgroundColor(Color.rgb(7, 10, 22));

        portrait = new VerityPortrait(this);
        FrameLayout.LayoutParams art = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        root.addView(portrait, art);

        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setGravity(Gravity.CENTER_HORIZONTAL);
        panel.setPadding(dp(18), dp(18), dp(18), dp(18));
        FrameLayout.LayoutParams panelParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.BOTTOM);
        panelParams.setMargins(dp(14), 0, dp(14), dp(16));
        panel.setBackgroundColor(0xDD101A31);

        TextView heading = label("LOBBY • VERITY", 21, Color.WHITE, true);
        heading.setGravity(Gravity.CENTER);
        panel.addView(heading);

        status = label("Estou aqui. Podemos conversar antes de partir.", 14,
                Color.rgb(190, 220, 255), false);
        status.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams statusParams = new LinearLayout.LayoutParams(-1, -2);
        statusParams.topMargin = dp(7);
        statusParams.bottomMargin = dp(9);
        panel.addView(status, statusParams);

        actions = new LinearLayout(this);
        actions.setOrientation(LinearLayout.VERTICAL);
        actions.setVisibility(View.GONE);

        Button talk = makeButton("MENSAGEM PARA VERITY", Color.rgb(82, 49, 128));
        actions.addView(talk, new LinearLayout.LayoutParams(-1, dp(50)));
        talk.setOnClickListener(v -> startActivity(new Intent(this, VeritySceneActivity.class)));

        Button wave = makeButton("ACENAR PARA VERITY", Color.rgb(35, 91, 126));
        LinearLayout.LayoutParams waveParams = new LinearLayout.LayoutParams(-1, dp(50));
        waveParams.topMargin = dp(7);
        actions.addView(wave, waveParams);
        wave.setOnClickListener(v -> {
            portrait.wave();
            status.setText("Verity percebeu seu gesto e acenou de volta.");
        });

        Button start = makeButton("COMEÇAR PARTIDA", Color.rgb(22, 116, 91));
        LinearLayout.LayoutParams startParams = new LinearLayout.LayoutParams(-1, dp(54));
        startParams.topMargin = dp(12);
        actions.addView(start, startParams);
        start.setOnClickListener(v -> startActivity(new Intent(this, NativeGameActivity.class)));

        panel.addView(actions);
        root.addView(panel, panelParams);

        intro = buildIntro();
        root.addView(intro, new FrameLayout.LayoutParams(-1, -1));
        setContentView(root);
        playIntro();
    }

    private View buildIntro() {
        FrameLayout layer = new FrameLayout(this);
        layer.setBackgroundColor(Color.rgb(5, 9, 20));
        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.setGravity(Gravity.CENTER);
        content.setPadding(dp(24), dp(24), dp(24), dp(24));

        TextView title = label("APOCALYPSE", 30, Color.WHITE, true);
        title.setGravity(Gravity.CENTER);
        content.addView(title);

        TextView subtitle = label("ANTES DA JORNADA, UMA ÚLTIMA PREPARAÇÃO", 12,
                Color.rgb(93, 196, 255), true);
        subtitle.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams sub = new LinearLayout.LayoutParams(-1, -2);
        sub.topMargin = dp(12);
        content.addView(subtitle, sub);

        TextView line = label("Verity: "Eu estava esperando você. Quer conversar um pouco antes de sair?"",
                16, Color.rgb(220, 230, 245), false);
        line.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(-1, -2);
        lp.topMargin = dp(28);
        content.addView(line, lp);

        Button skip = makeButton("PULAR APRESENTAÇÃO", Color.rgb(47, 67, 96));
        LinearLayout.LayoutParams bp = new LinearLayout.LayoutParams(-1, dp(50));
        bp.topMargin = dp(30);
        content.addView(skip, bp);
        skip.setOnClickListener(v -> finishIntro());

        layer.addView(content, new FrameLayout.LayoutParams(-1, -1));
        return layer;
    }

    private void playIntro() {
        intro.setAlpha(1f);
        entrance = ValueAnimator.ofFloat(0f, 1f);
        entrance.setDuration(2400);
        entrance.setInterpolator(new AccelerateDecelerateInterpolator());
        entrance.addUpdateListener(a -> {
            float p = (Float) a.getAnimatedValue();
            intro.setAlpha(1f - p);
        });
        entrance.addListener(new android.animation.AnimatorListenerAdapter() {
            @Override public void onAnimationEnd(android.animation.Animator animation) {
                finishIntro();
            }
        });
        entrance.start();
    }

    private void finishIntro() {
        if (lobbyReady) return;
        lobbyReady = true;
        if (entrance != null) entrance.cancel();
        root.removeView(intro);
        actions.setVisibility(View.VISIBLE);
        portrait.wave();
    }

    @Override protected void onDestroy() {
        if (entrance != null) entrance.cancel();
        super.onDestroy();
    }

    /** Ilustração 2D temporária e leve; arte 3D/rig fica para integração posterior no motor. */
    private static final class VerityPortrait extends View {
        private final Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        private float wave = 0f;
        private ValueAnimator waveAnimator;

        VerityPortrait(Activity activity) {
            super(activity);
            setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        }

        void wave() {
            if (waveAnimator != null) waveAnimator.cancel();
            waveAnimator = ValueAnimator.ofFloat(0f, 1f, 0f);
            waveAnimator.setDuration(850);
            waveAnimator.addUpdateListener(a -> {
                wave = (Float) a.getAnimatedValue();
                invalidate();
            });
            waveAnimator.start();
        }

        private void paint(Canvas c, int color) {
            p.setColor(color);
            p.setStyle(Paint.Style.FILL);
        }

        @Override protected void onDraw(Canvas c) {
            super.onDraw(c);
            float w = getWidth(), h = getHeight();
            p.setShader(new android.graphics.LinearGradient(0, 0, w, h,
                    new int[]{Color.rgb(12, 27, 54), Color.rgb(28, 20, 55), Color.rgb(7, 10, 22)},
                    null, android.graphics.Shader.TileMode.CLAMP));
            c.drawRect(0, 0, w, h, p);
            p.setShader(null);

            // Halo e silhueta ilustrada.
            p.setColor(0x3349BFFF);
            c.drawCircle(w * .5f, h * .36f, Math.min(w, h) * .29f, p);

            float cx = w * .5f, cy = h * .38f;
            paint(c, Color.rgb(235, 239, 248));
            c.drawOval(new RectF(cx - w*.105f, cy - h*.12f, cx + w*.105f, cy + h*.15f), p);
            paint(c, Color.rgb(226, 231, 243));
            c.drawOval(new RectF(cx - w*.13f, cy - h*.19f, cx + w*.13f, cy + h*.06f), p);

            // Cabelo claro, olhos e roupa azul-marinho.
            paint(c, Color.rgb(218, 229, 248));
            c.drawOval(new RectF(cx - w*.145f, cy - h*.225f, cx + w*.145f, cy - h*.035f), p);
            paint(c, Color.rgb(32, 43, 72));
            c.drawRoundRect(new RectF(cx - w*.12f, cy - h*.02f, cx + w*.12f, cy + h*.045f),
                    12, 12, p);
            paint(c, Color.rgb(22, 37, 67));
            c.drawRoundRect(new RectF(cx - w*.19f, cy + h*.10f, cx + w*.19f, cy + h*.43f),
                    24, 24, p);
            paint(c, Color.rgb(88, 190, 255));
            c.drawRect(cx - w*.012f, cy + h*.11f, cx + w*.012f, cy + h*.39f, p);

            paint(c, Color.rgb(30, 42, 70));
            c.drawCircle(cx - w*.045f, cy - h*.075f, w*.009f, p);
            c.drawCircle(cx + w*.045f, cy - h*.075f, w*.009f, p);

            // Braço que se move no gesto de acenar.
            c.save();
            c.rotate(-28f - 45f*wave, cx + w*.16f, cy + h*.16f);
            paint(c, Color.rgb(225, 231, 243));
            c.drawRoundRect(new RectF(cx + w*.12f, cy + h*.08f, cx + w*.18f,
                    cy + h*.28f), 12, 12, p);
            c.restore();

            p.setTextSize(Math.max(14f, w*.045f));
            p.setTypeface(Typeface.DEFAULT_BOLD);
            p.setColor(Color.WHITE);
            p.setTextAlign(Paint.Align.CENTER);
            c.drawText("VERITY", cx, h*.78f, p);
            p.setTextSize(Math.max(10f, w*.027f));
            p.setTypeface(Typeface.DEFAULT);
            p.setColor(Color.rgb(164, 205, 245));
            c.drawText("SUA COMPANHEIRA DE JORNADA", cx, h*.82f, p);
        }
    }
}
