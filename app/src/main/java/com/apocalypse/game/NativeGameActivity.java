package com.apocalypse.game;

import android.app.Activity;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.Toast;
import android.os.Handler;
import android.os.Looper;
import android.graphics.Color;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.content.SharedPreferences;
import android.speech.tts.TextToSpeech;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.net.HttpURLConnection;
import java.net.URL;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import org.json.JSONArray;
import org.json.JSONObject;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class NativeGameActivity extends Activity {
    private GameSurface surface;
    private Joystick joystick;
    private TextView hud;
    private TextView veritySpeech;
    private TextToSpeech verityTts;
    private boolean ttsReady=false;
    private boolean verityVoice=true;
    private SharedPreferences verityPrefs;
    private final ExecutorService verityIo=Executors.newSingleThreadExecutor();
    private volatile boolean ambientRequestRunning=false;
    private final Runnable ambientVerity = new Runnable() {
        @Override public void run() {
            if (isFinishing() || isDestroyed()) return;
            requestAmbientVerity();
            hudHandler.postDelayed(this, 90000L);
        }
    };
    private final Handler hudHandler = new Handler(Looper.getMainLooper());
    private final Runnable hudUpdater = new Runnable() { @Override public void run() { if (hud != null) hud.setText(NativeRenderer.getHud()); hudHandler.postDelayed(this, 300); } };

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setStatusBarColor(Color.BLACK);
        getWindow().setNavigationBarColor(Color.BLACK);
        verityPrefs=getSharedPreferences("apocalypse_settings",MODE_PRIVATE);
        verityVoice=verityPrefs.getBoolean("verity_voice",true);
        verityTts=new TextToSpeech(this,result->{
            if(result==TextToSpeech.SUCCESS){
                int language=verityTts.setLanguage(new Locale("pt","BR"));
                ttsReady=language!=TextToSpeech.LANG_MISSING_DATA&&language!=TextToSpeech.LANG_NOT_SUPPORTED;
            }
        });
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

        veritySpeech = new TextView(this);
        veritySpeech.setTextColor(Color.WHITE);
        veritySpeech.setTextSize(14);
        veritySpeech.setGravity(Gravity.CENTER);
        veritySpeech.setPadding(14,10,14,10);
        veritySpeech.setBackgroundColor(0xCC101B2A);
        veritySpeech.setVisibility(View.GONE);
        FrameLayout.LayoutParams speechParams = new FrameLayout.LayoutParams(
            FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.WRAP_CONTENT,
            Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL);
        speechParams.setMargins(180,0,180,(int)(150*getResources().getDisplayMetrics().density));
        frame.addView(veritySpeech,speechParams);

        Button voiceToggle = new Button(this);
        voiceToggle.setText(verityVoice ? "VERITY: VOZ ON" : "VERITY: VOZ OFF");
        FrameLayout.LayoutParams voiceParams = new FrameLayout.LayoutParams(-2,-2,Gravity.TOP|Gravity.END);
        voiceParams.setMargins(0,12,12,0);
        frame.addView(voiceToggle,voiceParams);
        voiceToggle.setOnClickListener(v->{
            verityVoice=!verityVoice;
            verityPrefs.edit().putBoolean("verity_voice",verityVoice).apply();
            voiceToggle.setText(verityVoice ? "VERITY: VOZ ON" : "VERITY: VOZ OFF");
            if(!verityVoice && verityTts!=null)verityTts.stop();
        });

        hud = new TextView(this);
        hud.setTextColor(Color.WHITE);
        hud.setTextSize(12);
        hud.setPadding(14, 8, 14, 8);
        hud.setBackgroundColor(0x99000000);
        FrameLayout.LayoutParams hudParams = new FrameLayout.LayoutParams(-2, -2, Gravity.TOP | Gravity.START);
        hudParams.setMargins(12, 64, 12, 0);
        frame.addView(hud, hudParams);

        Button shopToggle = new Button(this);
        shopToggle.setText("LOJA  $");
        FrameLayout.LayoutParams shopToggleParams = new FrameLayout.LayoutParams(-2,-2,Gravity.TOP|Gravity.END);
        shopToggleParams.setMargins(0,64,12,0);
        frame.addView(shopToggle,shopToggleParams);

        LinearLayout shopPanel = new LinearLayout(this);
        shopPanel.setOrientation(LinearLayout.VERTICAL);
        shopPanel.setPadding(12,12,12,12);
        shopPanel.setBackgroundColor(0xEE101B2A);
        shopPanel.setVisibility(View.GONE);
        FrameLayout.LayoutParams shopPanelParams = new FrameLayout.LayoutParams(
            (int)(250*getResources().getDisplayMetrics().density),-2,
            Gravity.CENTER|Gravity.END);
        shopPanelParams.setMargins(0,0,12,0);
        frame.addView(shopPanel,shopPanelParams);

        TextView shopTitle = new TextView(this);
        shopTitle.setText("LOJA DE SOBREVIVÊNCIA\nLimpe a onda para ganhar bônus de dinheiro e itens!");
        shopTitle.setTextColor(Color.WHITE);
        shopTitle.setTextSize(14);
        shopTitle.setPadding(4,4,4,8);
        shopPanel.addView(shopTitle);
        Button buyHeal = new Button(this);
        buyHeal.setText("CURA +40  •  $50");
        shopPanel.addView(buyHeal);
        buyHeal.setOnClickListener(v -> buyItem(0,"Cura comprada!"));
        Button buyPulse = new Button(this);
        buyPulse.setText("PULSO FORTE  •  $120");
        shopPanel.addView(buyPulse);
        buyPulse.setOnClickListener(v -> buyItem(1,"Pulso melhorado!"));
        Button buyArmor = new Button(this);
        buyArmor.setText("COLETE  •  $90");
        shopPanel.addView(buyArmor);
        buyArmor.setOnClickListener(v -> buyItem(2,"Colete melhorado!"));
        shopToggle.setOnClickListener(v -> {
            shopPanel.setVisibility(shopPanel.getVisibility()==View.VISIBLE?View.GONE:View.VISIBLE);
        });

        Button buyShuriken = new Button(this);
        buyShuriken.setText("DESBLOQUEAR SHURIKEN • $100");
        shopPanel.addView(buyShuriken);
        buyShuriken.setOnClickListener(v -> buyItem(3,"Shuriken desbloqueado!"));
        Button buyRifle = new Button(this);
        buyRifle.setText("DESBLOQUEAR RIFLE • $260");
        shopPanel.addView(buyRifle);
        buyRifle.setOnClickListener(v -> buyItem(4,"Rifle desbloqueado!"));
        Button buyShotgun = new Button(this);
        buyShotgun.setText("DESBLOQUEAR ESCOPETA • $220");
        shopPanel.addView(buyShotgun);
        buyShotgun.setOnClickListener(v -> buyItem(5,"Escopeta desbloqueada!"));
        Button ammoShuriken = new Button(this);
        ammoShuriken.setText("12 SHURIKENS • $45");
        shopPanel.addView(ammoShuriken);
        ammoShuriken.setOnClickListener(v -> buyItem(6,"Shurikens comprados!"));
        Button ammoRifle = new Button(this);
        ammoRifle.setText("30 MUNIÇÕES • $80");
        shopPanel.addView(ammoRifle);
        ammoRifle.setOnClickListener(v -> buyItem(7,"Munição comprada!"));
        Button ammoShotgun = new Button(this);
        ammoShotgun.setText("12 CARTUCHOS • $70");
        shopPanel.addView(ammoShotgun);
        ammoShotgun.setOnClickListener(v -> buyItem(8,"Cartuchos comprados!"));
        Button selectPulse = new Button(this);
        selectPulse.setText("EQUIPAR PULSO");
        shopPanel.addView(selectPulse);
        selectPulse.setOnClickListener(v -> selectWeapon(0));
        Button selectShuriken = new Button(this);
        selectShuriken.setText("EQUIPAR SHURIKEN");
        shopPanel.addView(selectShuriken);
        selectShuriken.setOnClickListener(v -> selectWeapon(1));
        Button selectRifle = new Button(this);
        selectRifle.setText("EQUIPAR RIFLE");
        shopPanel.addView(selectRifle);
        selectRifle.setOnClickListener(v -> selectWeapon(2));
        Button selectShotgun = new Button(this);
        selectShotgun.setText("EQUIPAR ESCOPETA");
        shopPanel.addView(selectShotgun);
        selectShotgun.setOnClickListener(v -> selectWeapon(3));
        Button pulse = new Button(this);
        pulse.setText("ATACAR");
        pulse.setOnClickListener(v -> NativeRenderer.attack());
        FrameLayout.LayoutParams pulseParams = new FrameLayout.LayoutParams(-2, -2, Gravity.BOTTOM | Gravity.END);
        pulseParams.setMargins(0, 0, 22, 38);
        frame.addView(pulse, pulseParams);

        joystick = new Joystick();
        int size = (int)(156 * getResources().getDisplayMetrics().density);
        FrameLayout.LayoutParams controls = new FrameLayout.LayoutParams(size, size, Gravity.BOTTOM | Gravity.START);
        controls.setMargins(20, 0, 0, 24);
        frame.addView(joystick, controls);
        setContentView(frame);
    }

    private void selectWeapon(int weapon) {
        boolean selected = NativeRenderer.selectWeapon(weapon);
        Toast.makeText(this, selected ? "Arma equipada!" : "Arma bloqueada ou sem munição.", Toast.LENGTH_SHORT).show();
        if (hud != null) hud.setText(NativeRenderer.getHud());
    }

    private void buyItem(int item,String successMessage) {
        boolean bought=NativeRenderer.purchase(item);
        Toast.makeText(this,bought?successMessage:"Dinheiro insuficiente ou limite atingido.",Toast.LENGTH_SHORT).show();
        if(hud!=null) hud.setText(NativeRenderer.getHud());
    }

    @Override protected void onResume() {
        super.onResume();
        if (surface != null) surface.onResume();
        hudHandler.post(hudUpdater);
        hudHandler.postDelayed(ambientVerity,25000L);
    }
    @Override protected void onPause() {
        hudHandler.removeCallbacks(hudUpdater);
        hudHandler.removeCallbacks(ambientVerity);
        if (verityTts != null) verityTts.stop();
        if (joystick != null) joystick.release();
        if (surface != null) surface.onPause();
        super.onPause();
    }
    @Override protected void onDestroy() {
        hudHandler.removeCallbacksAndMessages(null);
        if(verityTts!=null){verityTts.stop();verityTts.shutdown();}
        verityIo.shutdownNow();
        super.onDestroy();
    }

    private void requestAmbientVerity() {
        if(ambientRequestRunning || isFinishing() || isDestroyed()) return;
        String endpoint=verityPrefs.getString("verity_endpoint","").trim();
        String token=verityPrefs.getString("verity_gateway_token","").trim();
        if(endpoint.isEmpty() || token.isEmpty()) return;
        ambientRequestRunning=true;
        final String context=hud==null?"":hud.getText().toString();
        verityIo.execute(()->{
            HttpURLConnection connection=null;
            try {
                JSONArray messages=new JSONArray();
                messages.put(new JSONObject().put("role","system").put("content",
                    "Você é VERITY, companheira de sobrevivência do APOCALYPSE. Fale em português brasileiro. " +
                    "Inicie uma fala espontânea curta e natural (uma ou duas frases), útil e variada. " +
                    "Use somente o estado do jogo fornecido; não invente acontecimentos, locais ou ações. " +
                    "Não faça perguntas repetitivas, não narre como se estivesse vendo algo que não consta no estado. " +
                    "Não use formatação, listas ou emojis. Se não houver nada relevante, responda exatamente [SILENCIO]."));
                messages.put(new JSONObject().put("role","user").put("content",
                    "Estado atual da partida (HUD): "+context+
                    ". Gere uma fala espontânea que faça sentido agora."));
                JSONObject payload=new JSONObject();
                payload.put("model","apocalypse-verity");
                payload.put("messages",messages);
                payload.put("temperature",0.85);
                payload.put("max_tokens",90);
                payload.put("stream",false);
                connection=(HttpURLConnection)new URL(endpoint).openConnection();
                connection.setRequestMethod("POST");
                connection.setConnectTimeout(12000);
                connection.setReadTimeout(45000);
                connection.setDoOutput(true);
                connection.setRequestProperty("Content-Type","application/json");
                connection.setRequestProperty("Accept","application/json");
                connection.setRequestProperty("Authorization","Bearer "+token);
                try(OutputStream out=connection.getOutputStream()){
                    out.write(payload.toString().getBytes(StandardCharsets.UTF_8));
                }
                int status=connection.getResponseCode();
                java.io.InputStream stream=status>=200&&status<300?connection.getInputStream():connection.getErrorStream();
                StringBuilder raw=new StringBuilder();
                if(stream!=null){
                    try(BufferedReader reader=new BufferedReader(new InputStreamReader(stream,StandardCharsets.UTF_8))){
                        String line;while((line=reader.readLine())!=null)raw.append(line);
                    }
                }
                if(status<200||status>=300)throw new IllegalStateException("HTTP "+status);
                JSONObject response=new JSONObject(raw.toString());
                JSONArray choices=response.optJSONArray("choices");
                if(choices==null||choices.length()==0)throw new IllegalStateException("Resposta vazia");
                JSONObject message=choices.getJSONObject(0).optJSONObject("message");
                String line=message==null?"":message.optString("content","").trim();
                if(line.isEmpty()||line.equals("[SILENCIO]"))return;
                runOnUiThread(()->{
                    if(isFinishing()||isDestroyed())return;
                    veritySpeech.setText("VERITY  •  "+line);
                    veritySpeech.setVisibility(View.VISIBLE);
                    veritySpeech.removeCallbacks(hideVeritySpeech);
                    veritySpeech.postDelayed(hideVeritySpeech,12000L);
                    if(verityVoice&&ttsReady&&verityTts!=null)
                        verityTts.speak(line,TextToSpeech.QUEUE_FLUSH,null,"verity-ambient");
                });
            } catch(Exception ignored) {
                // Falhas temporárias da API não interrompem a partida nem exibem erros repetidos.
            } finally {
                if(connection!=null)connection.disconnect();
                ambientRequestRunning=false;
            }
        });
    }
    private final Runnable hideVeritySpeech=()->{
        if(veritySpeech!=null)veritySpeech.setVisibility(View.GONE);
    };

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
            setRenderer(new NativeRenderer(NativeGameActivity.this.getAssets()));
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        }
    }

    private static final class NativeRenderer implements GLSurfaceView.Renderer {
        static { System.loadLibrary("apocalypse_engine"); }
        private final AssetManager assets;
        NativeRenderer(AssetManager assets) { this.assets = assets; }
        private static native void nativeInit(AssetManager assets);
        private static native void nativeResize(int width, int height);
        private static native void nativeRender();
        private static native void nativeSetMove(float x, float z);
        private static native boolean nativeAttack();
        private static native boolean nativePurchase(int item);\n        private static native boolean nativeSelectWeapon(int weapon);
        private static native String nativeGetHud();
        static void attack(){ nativeAttack(); }
        static boolean purchase(int item){ return nativePurchase(item); }\n        static boolean selectWeapon(int weapon){ return nativeSelectWeapon(weapon); }
        static String getHud(){ return nativeGetHud(); }
        static void setMove(float x,float z){nativeSetMove(x,z);}
        @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) { nativeInit(assets); }
        @Override public void onSurfaceChanged(GL10 gl, int width, int height) { nativeResize(width, height); }
        @Override public void onDrawFrame(GL10 gl) { nativeRender(); }
    }
}
