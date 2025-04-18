# Trabalho Prático — Sistema de Controle de Smart Grids

**Execução:** Individual  

## 🧠 Introdução

Smart Grids são redes elétricas inteligentes que utilizam tecnologias de informação e comunicação para controle e automação de sistemas de energia. Elas operam com fluxo bidirecional de dados e são altamente distribuídas.

O sistema SCADA realiza coleta, supervisão e controle de dados em tempo real, sendo utilizado em infraestruturas críticas como usinas, redes de água e transporte público.

Neste trabalho, será desenvolvido um sistema cliente-servidor para simular a interação entre:
- **MTU (Main Terminal Unit)** – servidor
- **RTU (Remote Terminal Unit)** – cliente

## 📡 Protocolo

A comunicação é feita via **TCP**, com mensagens de até **500 bytes**, contendo apenas **letras, números e espaços** (sem acentos ou caracteres especiais). Toda mensagem termina com `\n`.

## ✉️ Especificação das Mensagens

| Mensagem | Parâmetros | Descrição |
|----------|------------|-----------|
| `INS_REQ` | sensorId cor ten efic_energ | Inicia sensor |
| `REM_REQ` | sensorId | Remove sensor |
| `CH_REQ` | sensorId cor ten efic_energ | Altera valores |
| `SEN_REQ` | sensorId | Consulta sensor |
| `VAL_REQ` | – | Consulta todos os sensores |
| `SEN_RES` / `VAL_RES` | dados do sensor | Resposta |
| `ERROR` | código | Mensagem de erro (`01`: sensor não instalado, `02`: nenhum sensor, etc.) |
| `OK` | código | Confirmação (`01`: instalação bem-sucedida, etc.) |

## ⚙️ Funcionalidades

1. **Ligar Sensor**
   - Comando: `install param sensorId cor ten efic_energ`
   - Validação → Envio → Resposta (`OK` ou `ERROR`)
2. **Desligar Sensor**
   - Comando: `remove sensorId`
   - Envio → Verificação → Remoção → Resposta
3. **Atualizar Sensor**
   - Comando: `change param sensorId cor ten efic_energ`
   - Validação → Envio → Atualização → Resposta
4. **Consultar Sensor**
   - Comando: `show value sensorId`
   - Envio → Verificação → Resposta com dados
5. **Consultar Todos os Sensores**
   - Comando: `show values`
   - Envio → Resposta com todos os dados

## 🛠️ Implementação

- Linguagem: **C**
- Comunicação via **sockets TCP (POSIX)**
- Interface: `socket(AF_INET/SOCK_STREAM, 0)`
- Uso das funções: `send`, `recv`
- Término do servidor: ao receber `"kill"`
- Limite: mensagens de no máximo 500 bytes

### Execução

```bash
# Servidor
./server v4 90900

# Cliente
./client 127.0.0.1 90900
