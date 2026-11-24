package com.apocalypse.game;

import android.app.Activity;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import org.json.JSONArray;
import org.json.JSONObject;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class VeritySceneActivity extends Activity {
    private final ExecutorService io=Executors.newSingleThreadExecutor();
    private LinearLayout messages;
    private EditText input;
    private TextView status;
    private TextToSpeech tts;
    private boolean ttsReady=false;
    private boolean voiceEnabled=true;
    private SharedPreferences prefs;
    private final JSONArray history=new JSONArray();
    private static final String SYSTEM_PROMPT = "Você é VERITY, companheira inteligente do jogador no jogo APOCALYPSE. Fale sempre em português brasileiro natural, com personalidade consistente, respostas contextuais e sem repetir bordões. Você pode iniciar a conversa espontaneamente, fazer perguntas relevantes e comentar o contexto que o jogador fornecer. Não afirme que viu acontecimentos do jogo se eles não foram enviados no contexto. Seja acolhedora, objetiva e útil.";

    private int dp(float v){return (int)(v*getResources().getDisplayMetrics().density+.5f);}
    private TextView label(String s,int size,int color){TextView t=new TextView(this);t.setText(s);t.setTextSize(size);t.setTextColor(color);return t;}
    private void addMessage(String who,String value,boolean user){
        TextView t=label(who+"\n"+value,15,user?Color.WHITE:Color.rgb(175,220,255));
        t.setPadding(dp(12),dp(10),dp(12),dp(10));
        t.setBackgroundColor(user?Color.rgb(35,53,78):Color.rgb(35,28,57));
        LinearLayout.LayoutParams p=new LinearLayout.LayoutParams(-1,-2);p.bottomMargin=dp(8);messages.addView(t,p);
    }
    private void setStatus(String s){runOnUiThread(()->status.setText(s));}
    @Override public void onCreate(Bundle state){
        super.onCreate(state); getWindow().setStatusBarColor(Color.rgb(8,10,20));getWindow().setNavigationBarColor(Color.rgb(8,10,20));
        prefs=getSharedPreferences("apocalypse_settings",MODE_PRIVATE);
        voiceEnabled=prefs.getBoolean("verity_voice",true);
        tts=new TextToSpeech(this,result->{if(result==TextToSpeech.SUCCESS){int r=tts.setLanguage(new Locale("pt","BR"));ttsReady=r!=TextToSpeech.LANG_MISSING_DATA&&r!=TextToSpeech.LANG_NOT_SUPPORTED;}});
        LinearLayout root=new LinearLayout(this);root.setOrientation(LinearLayout.VERTICAL);root.setPadding(dp(16),dp(18),dp(16),dp(12));root.setBackgroundColor(Color.rgb(8,10,20));
        TextView title=label("VERITY",26,Color.WHITE);title.setGravity(Gravity.CENTER);root.addView(title);
        TextView subtitle=label("CENA DE CONVERSA • IA GENERATIVA",11,Color.rgb(90,190,255));subtitle.setGravity(Gravity.CENTER);root.addView(subtitle);
        status=label("Aguardando configuração da IA…",12,Color.LTGRAY);status.setPadding(0,dp(8),0,dp(8));root.addView(status);
        ScrollView scroll=new ScrollView(this);messages=new LinearLayout(this);messages.setOrientation(LinearLayout.VERTICAL);scroll.addView(messages);
        root.addView(scroll,new LinearLayout.LayoutParams(-1,0,1));
        LinearLayout controls=new LinearLayout(this);controls.setOrientation(LinearLayout.VERTICAL);
        input=new EditText(this);input.setSingleLine(false);input.setMinLines(1);input.setMaxLines(3);input.setHint("Converse com a VERITY…");input.setTextColor(Color.WHITE);input.setHintTextColor(Color.GRAY);controls.addView(input);
        Button send=new Button(this);send.setText("ENVIAR");controls.addView(send);send.setOnClickListener(v->sendMessage());
        Button voice=new Button(this);voice.setText(voiceEnabled?"VOZ: LIGADA":"VOZ: DESLIGADA");controls.addView(voice);
        voice.setOnClickListener(v->{voiceEnabled=!voiceEnabled;prefs.edit().putBoolean("verity_voice",voiceEnabled).apply();voice.setText(voiceEnabled?"VOZ: LIGADA":"VOZ: DESLIGADA");});
        Button endpoint=new Button(this);endpoint.setText("CONFIGURAR IA (ENDPOINT COMPATÍVEL)");controls.addView(endpoint);
        endpoint.setOnClickListener(v->configureEndpoint());
        Button testConnection=new Button(this);testConnection.setText("TESTAR CONEXÃO DA IA");controls.addView(testConnection);
        testConnection.setOnClickListener(v->testGateway());
        Button back=new Button(this);back.setText("VOLTAR AO LOBBY");controls.addView(back);back.setOnClickListener(v->finish());
        root.addView(controls);setContentView(root);
        try { history.put(new JSONObject().put("role","system").put("content",SYSTEM_PROMPT)); } catch(Exception ignored) {}
        String configured=prefs.getString("verity_endpoint","").trim();
        if(configured.isEmpty()) {
            addMessage("SISTEMA","Configure um endpoint de IA generativa para iniciar a conversa. Não há falas repetidas programadas.",false);
            setStatus("IA ainda não configurada • voz PT-BR disponível");
        } else {
            setStatus("VERITY está iniciando a conversa…");
            requestVerity(null);
        }
        if(getIntent().getBooleanExtra("settings",false))configureEndpoint();
    }
    private String normalizeEndpoint(String raw) throws Exception {
        String value=raw.trim();
        if(value.isEmpty())throw new IllegalArgumentException("Informe o endereço do Render.");
        if(!value.contains("://"))value="https://"+value;
        URL parsed=new URL(value);
        if(!"https".equalsIgnoreCase(parsed.getProtocol())&&!"http".equalsIgnoreCase(parsed.getProtocol()))
            throw new IllegalArgumentException("Use um endereço HTTPS do Render.");
        if(parsed.getHost().isEmpty())throw new IllegalArgumentException("Endereço inválido.");
        String clean=value.replaceAll("/+$","");
        if(clean.endsWith("/health"))clean=clean.substring(0,clean.length()-7);
        if(!clean.endsWith("/v1/chat/completions"))clean+="/v1/chat/completions";
        return clean;
    }
    private String healthUrl(String endpoint){
        return endpoint.substring(0,endpoint.length()-"/v1/chat/completions".length())+"/health";
    }
    private String readBody(java.io.InputStream stream) throws Exception {
        if(stream==null)return "";
        BufferedReader reader=new BufferedReader(new InputStreamReader(stream,StandardCharsets.UTF_8));
        StringBuilder raw=new StringBuilder();String line;
        while((line=reader.readLine())!=null)raw.append(line);
        reader.close();return raw.toString();
    }
    private void configureEndpoint(){
        LinearLayout form=new LinearLayout(this);
        form.setOrientation(LinearLayout.VERTICAL);
        form.setPadding(dp(18),dp(8),dp(18),dp(4));
        final EditText field=new EditText(this);
        field.setSingleLine(true);
        field.setText(prefs.getString("verity_endpoint",""));
        field.setHint("https://seu-servico.onrender.com");
        form.addView(field);
        final EditText token=new EditText(this);
        token.setSingleLine(true);
        token.setInputType(android.text.InputType.TYPE_CLASS_TEXT|android.text.InputType.TYPE_TEXT_VARIATION_PASSWORD);
        token.setText(prefs.getString("verity_gateway_token",""));
        token.setHint("VERITY_GATEWAY_TOKEN");
        form.addView(token);
        new android.app.AlertDialog.Builder(this).setTitle("Conectar API da VERITY")
          .setMessage("Cole o domínio do serviço Render ou a URL completa. Informe o VERITY_GATEWAY_TOKEN do mesmo serviço. A chave do provedor fica somente no Render.")
          .setView(form).setNegativeButton("Cancelar",null).setPositiveButton("Salvar",(d,w)->{
              try{
                  String endpoint=normalizeEndpoint(field.getText().toString());
                  String accessToken=token.getText().toString().trim();
                  prefs.edit().putString("verity_endpoint",endpoint)
                      .putString("verity_gateway_token",accessToken).apply();
                  setStatus("Endereço salvo. Toque em TESTAR CONEXÃO DA IA.");
              }catch(Exception e){setStatus("Configuração inválida: "+e.getMessage());}
          }).show();
    }
    private void testGateway(){
        String endpoint=prefs.getString("verity_endpoint","").trim();
        if(endpoint.isEmpty()){setStatus("Configure primeiro o domínio do Render.");return;}
        setStatus("Verificando serviço e variáveis da VERITY…");
        io.execute(()->{
            HttpURLConnection c=null;
            try{
                c=(HttpURLConnection)new URL(healthUrl(endpoint)).openConnection();
                c.setRequestMethod("GET");c.setConnectTimeout(12000);c.setReadTimeout(15000);
                int code=c.getResponseCode();
                String raw=readBody(code>=200&&code<300?c.getInputStream():c.getErrorStream());
                if(code<200||code>=300)throw new IllegalStateException("Health HTTP "+code+": "+raw);
                JSONObject h=new JSONObject(raw);
                boolean model=h.optBoolean("model_configured",false);
                boolean key=h.optBoolean("provider_key_configured",false);
                boolean auth=h.optBoolean("gateway_auth_configured",false);
                String result="Gateway online • modelo/URL: "+(model?"OK":"FALTA")+
                    " • chave do provedor: "+(key?"OK":"FALTA")+
                    " • token do gateway: "+(auth?"OK":"FALTA");
                setStatus(result);
                runOnUiThread(()->addMessage("DIAGNÓSTICO",result,false));
            }catch(Exception e){
                String detail=e.getMessage()==null?e.getClass().getSimpleName():e.getMessage();
                setStatus("Teste da IA falhou: "+detail);
                runOnUiThread(()->addMessage("DIAGNÓSTICO",detail,false));
            }finally{if(c!=null)c.disconnect();}
        });
    }
    private void sendMessage(){
        String message=input.getText().toString().trim();if(message.isEmpty())return;
        input.setText("");addMessage("VOCÊ",message,true);
        try{history.put(new JSONObject().put("role","user").put("content",message));}catch(Exception ignored){}
        requestVerity(message);
    }
    private void requestVerity(String userMessage){
        String endpoint=prefs.getString("verity_endpoint","").trim();
        String gatewayToken=prefs.getString("verity_gateway_token","").trim();
        if(endpoint.isEmpty()){setStatus("Configure o endereço do gateway da VERITY.");return;}
        if(gatewayToken.isEmpty()){setStatus("Falta o VERITY_GATEWAY_TOKEN do Render. Abra CONFIGURAR IA.");return;}
        setStatus("VERITY está pensando…");
        io.execute(()->{
            HttpURLConnection c=null;
            try{
                JSONObject body=new JSONObject();body.put("model","apocalypse-verity");
                body.put("messages",history);body.put("temperature",0.75);body.put("stream",false);
                c=(HttpURLConnection)new URL(endpoint).openConnection();
                c.setRequestMethod("POST");c.setConnectTimeout(15000);c.setReadTimeout(80000);
                c.setDoOutput(true);c.setRequestProperty("Content-Type","application/json");
                c.setRequestProperty("Accept","application/json");
                c.setRequestProperty("Authorization","Bearer "+gatewayToken);
                try(OutputStream os=c.getOutputStream()){os.write(body.toString().getBytes(StandardCharsets.UTF_8));}
                int code=c.getResponseCode();
                String raw=readBody(code>=200&&code<300?c.getInputStream():c.getErrorStream());
                if(code<200||code>=300){
                    String detail="HTTP "+code;
                    try{
                        JSONObject err=new JSONObject(raw).optJSONObject("error");
                        if(err!=null){
                            String msg=err.optString("message","");
                            if(!msg.isEmpty())detail+=" • "+msg;
                            int provider=err.optInt("provider_status",0);
                            if(provider>0)detail+=" (provedor HTTP "+provider+")";
                        }else if(!raw.isEmpty())detail+=" • "+raw.substring(0,Math.min(240,raw.length()));
                    }catch(Exception ignored){if(!raw.isEmpty())detail+=" • "+raw.substring(0,Math.min(240,raw.length()));}
                    if(code==401)detail+=" • confira VERITY_GATEWAY_TOKEN.";
                    if(code==503)detail+=" • confira VERITY_UPSTREAM_URL, VERITY_API_KEY e VERITY_MODEL no Render.";
                    throw new IllegalStateException(detail);
                }
                JSONObject response=new JSONObject(raw);
                JSONArray choices=response.optJSONArray("choices");
                if(choices==null||choices.length()==0)throw new IllegalStateException("O provedor não retornou choices. Resposta: "+raw.substring(0,Math.min(220,raw.length())));
                JSONObject message=choices.getJSONObject(0).optJSONObject("message");
                String answer=message==null?"":message.optString("content","").trim();
                if(answer.isEmpty())throw new IllegalStateException("A IA retornou conteúdo vazio ou formato incompatível.");
                history.put(new JSONObject().put("role","assistant").put("content",answer));
                runOnUiThread(()->{addMessage("VERITY",answer,false);setStatus("VERITY respondeu por IA generativa.");if(voiceEnabled&&ttsReady)tts.speak(answer,TextToSpeech.QUEUE_ADD,null,"verity-response");});
            }catch(Exception e){
                String detail=e.getMessage()==null?e.getClass().getSimpleName():e.getMessage();
                setStatus("Falha na IA: "+detail);
                runOnUiThread(()->addMessage("ERRO DA IA",detail,false));
            }finally{if(c!=null)c.disconnect();}
        });
    }
    @Override protected void onDestroy(){if(tts!=null){tts.stop();tts.shutdown();}io.shutdownNow();super.onDestroy();}
}
