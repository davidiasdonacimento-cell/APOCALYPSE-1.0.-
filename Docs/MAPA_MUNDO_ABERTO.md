# Mapa do mundo aberto — APOCALYPSE 1.0

## Objetivo
Construir um mundo aberto de sobrevivência zumbi para Android, dividido em regiões conectadas. O mapa será carregado por setores (chunks) para controlar memória e desempenho no aparelho.

## Regiões principais

1. **Metrópole abandonada** — centro urbano, hospital, comércio, avenidas, bairros residenciais e áreas de serviço.
2. **Floresta contaminada** — mata densa, estradas secundárias, clareiras e construções isoladas.
3. **Zona militar** — complexo cercado, alojamentos, hangares e postos de observação abandonados.
4. **Cidade rural** — casas, fazendas, celeiros, pequenas lojas e estradas de terra.

## Conexões do mundo

- A metrópole conecta-se à floresta por uma rodovia danificada.
- A floresta conecta-se à zona militar por uma estrada de serviço.
- A floresta conecta-se à cidade rural por uma ponte e estrada secundária.
- A cidade rural possui uma rota alternativa de retorno à metrópole.

## Layout lógico inicial (não representa escala final)

```text
                 [ ZONA MILITAR ]
                        |
                 Estrada de serviço
                        |
[ METRÓPOLE ] -- [ FLORESTA CONTAMINADA ] -- [ CIDADE RURAL ]
       \_________________________________________/
                  Rodovia alternativa
```

## Regras técnicas para Android

- Não carregar o mundo inteiro de uma vez.
- Dividir cada região em setores/chunks com coordenadas próprias.
- Carregar setores próximos ao jogador e descarregar os distantes.
- Separar dados do mapa, renderização, colisões e entidades.
- Usar níveis de detalhe e limites de distância para objetos.
- Manter pontos de entrada/saída entre regiões para transições controladas.
- Salvar alterações importantes do mundo sem serializar geometria estática desnecessariamente.

## Ordem de construção

1. Bloco de terreno e limites da primeira região.
2. Ruas principais e pontos de referência da metrópole.
3. Quarteirões e edifícios modulares.
4. Colisões e navegação do personagem.
5. Sistema de chunks e conexões entre regiões.
6. Vegetação, iluminação, ambientação e otimização.

## Escopo da primeira entrega

A primeira entrega técnica será apenas a base espacial da metrópole: terreno, ruas principais, limites e pontos de referência. Não será declarada como mapa jogável até existir renderização integrada, colisão e teste no Android.

## Incremento de expansão territorial — 24/11/2026
- Terreno procedural ampliado para uma área de aproximadamente 224 × 224 unidades, com malha de 144 × 144 células.
- Quatro regiões agora possuem geometria procedural conectada por corredores viários: metrópole, floresta contaminada, zona militar e povoado rural.
- Floresta: árvores orgânicas distribuídas, clareiras e posto florestal.
- Zona militar: hangar, alojamentos, perímetro com postes de cerca e torre de observação.
- Região rural: casas, anexos, árvores, fileiras de cultivo e canal de água.
- Foi adicionada uma clínica térrea com entrada física aberta, piso e divisórias/mobiliário internos. É o primeiro interior acessível previsto para teste.
- Limites de movimento e distância de projeção foram ampliados para permitir alcançar as novas regiões.
- Todo esse conteúdo ainda é geometria procedural com cores por vértice; não são modelos artísticos externos nem texturas reais de alta resolução.
- Chunks, streaming de regiões, materiais/texturas PBR, colisões de vegetação/objetos e teste físico no Android continuam pendentes.

## Estado

- Planejamento das quatro regiões: documentado.
- Conexões entre regiões: documentadas.
- Sistema de chunks: especificado, ainda não implementado.
- Terreno 3D e blockout urbano: presentes no renderer nativo.
- Primeiro distrito recebeu fachadas com janelas/portas, variação de edifícios, anexo hospitalar, caminhos, postes, carros abandonados, marcações viárias, entulho e cercas procedurais.
- Incremento orgânico: árvores com troncos cônicos e copas elipsoidais trianguladas; luminárias com postes cilíndricos e globos; pneus arredondados; sobrevivente e zumbis com volumes elipsoidais em vez de corpos inteiros em blocos.
- Essas malhas continuam sendo geometria procedural estilizada; não equivalem a personagens escaneados, modelos artísticos externos ou materiais PBR.
- Ainda não há modelos artísticos externos, texturas PBR, interiores exploráveis, colisão do cenário ou chunks; o mapa continua sendo uma área procedural em evolução.
- Renderização após a nova expansão, compilação CI e teste no Android: pendentes.


## Colisão do primeiro distrito — 24/11/2026
- As construções criadas pela função de edifícios registram volumes físicos AABB separados da malha visual.
- O personagem testa colisão circular contra esses volumes e resolve movimento nos eixos X/Z separadamente, permitindo deslizar ao longo das fachadas.
- Os limites externos continuam ativos.
- Esta etapa cobre edifícios registrados pelo gerador; ainda não representa colisão completa de carros, árvores, cercas, destroços ou interiores.
- Compilação e teste físico no Android ainda precisam ser confirmados.


## Materiais UV — 24/11/2026
- O terreno e superfícies planas passaram a transportar UVs e IDs de material para o shader.
- Materiais iniciais: vegetação, concreto e asfalto, com repetição controlada por escala espacial e mipmaps.
- Texturas iniciais são padrões procedurais 64 × 64 gerados em memória; não representam ainda materiais fotográficos de alta resolução.
- Modelos externos com UVs, texturas artísticas reais e materiais PBR continuam pendentes.

## Primeiro modelo OBJ renderizado com materiais PBR — 24/11/2026
- O pacote de assets da primeira região inclui `models/apocalypse_hospital.obj` e seu MTL, como prova de integração do importador no mundo.
- O modelo é carregado pelo `AssetManager`, enviado a buffers OpenGL e renderizado após o terreno/cenário procedural.
- Concreto, vidro e metal usam parâmetros de albedo, rugosidade e metalicidade; o shader aplica normal mapping e iluminação Cook–Torrance/GGX.
- O pipeline aceita mapas de textura declarados no material; quando ausentes, usa mapas compactos gerados em memória. Texturas fotográficas/artísticas de alta resolução ainda não foram entregues.
- O hospital importado é um asset inicial de integração; os demais prédios, interiores e regiões continuam aguardando substituição gradual por modelos artísticos.
- Teste visual no Android e validação de desempenho continuam pendentes.
