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

## Estado

- Planejamento das quatro regiões: documentado.
- Conexões entre regiões: documentadas.
- Sistema de chunks: especificado, ainda não implementado.
- Terreno 3D e blockout urbano: presentes no renderer nativo.
- Primeiro distrito recebeu fachadas com janelas/portas, variação de edifícios, anexo hospitalar, caminhos e árvores do parque, postes, carros abandonados, marcações viárias, entulho e cercas procedurais.
- Ainda não há modelos artísticos externos, texturas PBR, interiores exploráveis, colisão do cenário ou chunks; o mapa continua sendo uma área procedural de primeira etapa.
- Renderização após a nova expansão, compilação CI e teste no Android: pendentes.
