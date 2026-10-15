
# ESP32 IoT Control Center

Projeto educacional para aprender Internet das Coisas (IoT) usando ESP32, Arduino IDE, C++, HTML, CSS, JavaScript, HTTP e Git.

O sistema evolui gradualmente, começando por um servidor web local e avançando para monitoramento de sensores e automação.

## Objetivos

- Aprender a criar uma rede Wi-Fi independente com o ESP32.
- Hospedar uma interface web no próprio dispositivo.
- Controlar GPIOs pelo navegador.
- Ler sensores e disponibilizar dados via HTTP e JSON.
- Desenvolver interfaces web para sistemas embarcados.
- Aprender Git e GitHub por meio de um projeto prático.
- Documentar cada etapa de desenvolvimento.

## Arquitetura

O ESP32 funciona como ponto de acesso Wi-Fi e servidor web. Um celular ou computador conecta-se diretamente ao dispositivo, sem necessidade de roteador ou acesso à Internet.

## Versões

### V1 — Servidor web e controle de LED

- Criação de rede Wi-Fi própria.
- Servidor HTTP local.
- Interface web para ligar e desligar um LED.
- Controle de GPIO pelo navegador.

### V2 — Monitoramento de sensores

- Sensor DHT22 para temperatura e umidade.
- Potenciômetro conectado a uma entrada ADC.
- Endpoint HTTP `/data` com resposta JSON.
- Atualização dos valores no navegador com JavaScript.

### V3 — Planejada

Aprimoramentos na comunicação e na interface, definidos após a conclusão e os testes da V2.

## Estrutura do repositório

- `firmware/`: códigos das versões do ESP32.
- `docs/`: documentação técnica e registros de aprendizado.
- `README.md`: apresentação geral do projeto.

## Hardware

- Placa ESP32 compatível com o projeto.
- Sensor DHT22.
- Potenciômetro.
- LED e resistor limitador de corrente.
- Protoboard e jumpers.

## Software

- Arduino IDE.
- Pacote de placas ESP32 da Espressif.
- Biblioteca DHT sensor library, de Adafruit.
- Biblioteca Adafruit Unified Sensor.
- Git.

## Rede local

- SSID de desenvolvimento: `ESP32-IOT`
- Endereço local esperado: `192.168.4.1`

A rede criada pelo ESP32 não depende de Internet. Configure senhas próprias para uso real e não publique credenciais pessoais ou segredos no repositório.

## Documentação

Consulte os arquivos em `docs/` para conhecer a montagem, o funcionamento, os testes e os conceitos aprendidos em cada versão.

## Status

Projeto em desenvolvimento para fins educacionais.