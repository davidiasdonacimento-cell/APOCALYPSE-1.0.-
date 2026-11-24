package com.apocalypse.game;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

public final class MainActivity extends Activity {
    private int dp(float v) { return (int)(v * getResources().getDisplayMetrics().density + .5f); }
    private GradientDrawable panel(int color, int stroke) {
        GradientDrawable d = new GradientDrawable(GradientDrawable.Orientation.TL_BR,
                new int[]{color, Color.rgb(12, 17, 32)});
        d.setCornerRadius(dp(18));
        d.setStroke(dp(1), stroke);
        return d;
    }
    private TextView text(String value, int size, int color, boolean bold) {
        TextView t = new TextView(this);
        t.setText(value); t.setTextSize(size); t.setTextColor(color);
        if (bold) t.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        return t;
    }
    private Button button(String label, int color) {
        Button b = new Button(this); b.setText(label); b.setTextColor(Color.WHITE);
        b.setAllCaps(false); b.setTextSize(16);
        b.setBackground(panel(color, Color.rgb(65, 111, 180)));
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(-1, dp(54));
        p.bottomMargin = dp(10); b.setLayoutParams(p); return b;
    }
    private void section(LinearLayout root, String title, String subtitle) {
        LinearLayout card = new LinearLayout(this); card.setOrientation(LinearLayout.VERTICAL);
        card.setPadding(dp(16),dp(14),dp(16),dp(14)); card.setBackground(panel(Color.rgb(24,32,55),Color.rgb(45,72,112)));
        TextView h=text(title,18,Color.WHITE,true); TextView s=text(subtitle,13,Color.LTGRAY,false);
        card.addView(h); LinearLayout.LayoutParams sp=new LinearLayout.LayoutParams(-1,-2); sp.topMargin=dp(5); card.addView(s,sp);
        LinearLayout.LayoutParams cp=new LinearLayout.LayoutParams(-1,-2); cp.bottomMargin=dp(12); root.addView(card,cp);
    }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setStatusBarColor(Color.rgb(7,10,20));
        getWindow().setNavigationBarColor(Color.rgb(7,10,20));
        ScrollView scroll=new ScrollView(this); scroll.setBackgroundColor(Color.rgb(7,10,20));
        LinearLayout root=new LinearLayout(this); root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(dp(20),dp(28),dp(20),dp(24)); scroll.addView(root);
        TextView logo=text("APOCALYPSE",30,Color.WHITE,true); logo.setGravity(Gravity.CENTER);
        root.addView(logo);
        TextView sub=text("SOBREVIVA. EXPLORE. DESCUBRA.",12,Color.rgb(83,190,255),true);
        sub.setGravity(Gravity.CENTER); LinearLayout.LayoutParams sm=new LinearLayout.LayoutParams(-1,-2); sm.bottomMargin=dp(22); root.addView(sub,sm);
        section(root,"LOBBY PRINCIPAL","Sua jornada ainda não começou. Prepare-se antes de entrar no mundo.");
        Button start=button("▶  INICIAR JOGO",Color.rgb(22,93,155));
        root.addView(start); start.setOnClickListener(v -> startActivity(new Intent(this,VeritySceneActivity.class)));
        Button resume=button("CONTINUAR",Color.rgb(47,57,78)); root.addView(resume);
        resume.setOnClickListener(v -> Toast.makeText(this,"Ainda não existe progresso salvo.",Toast.LENGTH_SHORT).show());
        section(root,"VERITY • COMPANHEIRA IA","Converse com ela antes da partida. As respostas devem ser geradas por IA e narradas por voz.");
        Button talk=button("CONVERSAR COM VERITY",Color.rgb(82,49,128)); root.addView(talk);
        talk.setOnClickListener(v -> startActivity(new Intent(this,VeritySceneActivity.class)));
        section(root,"SUA JORNADA","Acesso rápido às áreas do lobby. Recursos serão conectados às telas funcionais nas próximas etapas.");
        Button profile=button("PERFIL DO JOGADOR",Color.rgb(38,56,82)); root.addView(profile);
        profile.setOnClickListener(v -> Toast.makeText(this,"Perfil local: implementação pendente.",Toast.LENGTH_SHORT).show());
        Button inventory=button("INVENTÁRIO E EQUIPAMENTOS",Color.rgb(38,56,82)); root.addView(inventory);
        inventory.setOnClickListener(v -> Toast.makeText(this,"Inventário: implementação pendente.",Toast.LENGTH_SHORT).show());
        Button settings=button("CONFIGURAÇÕES",Color.rgb(38,56,82)); root.addView(settings);
        settings.setOnClickListener(v -> startActivity(new Intent(this,VeritySceneActivity.class).putExtra("settings",true)));
        Button customize=button("PERSONALIZAR",Color.rgb(38,56,82)); root.addView(customize);
        customize.setOnClickListener(v -> Toast.makeText(this,"Personalização: implementação pendente.",Toast.LENGTH_SHORT).show());
        TextView footer=text("APOCALYPSE 1.0  •  ANDROID",11,Color.GRAY,false); footer.setGravity(Gravity.CENTER);
        root.addView(footer);
        setContentView(scroll);
    }
}
