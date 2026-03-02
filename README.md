# 🌐 Sistema IoT de Monitoreo Ambiental

## 🔍 **Descripción del Proyecto**

Sistema completo de monitoreo IoT que combina sensores ESP32 con servidor Django para análisis en tiempo real de variables ambientales (temperatura, humedad y luminosidad) con sistema de alertas automatizado.

**Tecnologías**: Django 4.0.3, ESP32, MQTT, PostgreSQL, DHT11, LDR, OLED SH1106

---

## 📊 **Funcionalidades del Sistema**

### **🔧 Backend Django**
- ✅ **API REST** para gestión de datos IoT
- ✅ **Dashboard web** en tiempo real 
- ✅ **Base de datos** PostgreSQL para almacenamiento
- ✅ **Sistema de alertas** automatizado cada 3 segundos
- ✅ **Comunicación MQTT** bidireccional
- ✅ **Análisis estadístico** de datos históricos

### **📟 Hardware ESP32**
- ✅ **Sensores integrados**: DHT11, LDR, OLED SH1106
- ✅ **Comunicación WiFi** y MQTT
- ✅ **Sistema de alertas visuales** con patrones LED específicos
- ✅ **Pantalla OLED** para monitoreo local
- ✅ **Reconexión automática** y manejo robusto de errores

---

## 🚀 **Instalación y Configuración**

### **1. Configurar Entorno Django**

```bash
# Clonar el repositorio
cd IOTMonitoringServer

# Crear entorno virtual
python -m venv venv
source venv/bin/activate  # Linux/Mac
# o
venv\Scripts\activate     # Windows

# Instalar dependencias
pip install -r requirements.txt

# Configurar base de datos
python manage.py migrate

# Crear superusuario
python manage.py createsuperuser
```

### **2. Configurar Hardware ESP32**

#### **Componentes requeridos:**
- ESP32 DevKit
- Sensor DHT11 (pin 4)
- LDR + resistencia 10kΩ (pin 15)
- Pantalla OLED SH1106 (I2C: SDA=22, SCL=21)
- LED integrado (pin 2)

#### **Librerías Arduino IDE:**
```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
```

#### **Configuración de red:**
```cpp
const char ssid[] = "TU_WIFI";
const char pass[] = "TU_PASSWORD";
const char MQTT_HOST[] = "32.192.84.37";  // Cambiar por tu broker
const int MQTT_PORT = 8082;
```

### **3. Broker MQTT**

```bash
# Instalar Mosquitto (Ubuntu/Debian)
sudo apt update
sudo apt install mosquitto mosquitto-clients

# Iniciar servicio
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

---

## ▶️ **Ejecución del Sistema**

### **Ejecutar en 3 terminales simultáneamente:**

#### **Terminal 1: Servidor Web**
```bash
python manage.py runserver
# Acceder a: http://localhost:8000
```

#### **Terminal 2: Servicio MQTT**
```bash
python manage.py start_mqtt
# Recibe y procesa datos de sensores
```

#### **Terminal 3: Monitor de Alertas**
```bash
python manage.py start_control
# Análisis automático cada 3 segundos
```

### **Subir código a ESP32:**
1. Abrir `hardware/iot_device_with_event_processing.ino` en Arduino IDE
2. Configurar WiFi y MQTT en el código
3. Subir a ESP32
4. Abrir Monitor Serial (115200 baud)

---

## 📡 **Comunicación MQTT**

### **Tópicos configurados:**
```bash
# Publicación (ESP32 → Servidor)
colombia/cundinamarca/bogota/jn.cordobap1/out

# Suscripción (Servidor → ESP32)  
colombia/cundinamarca/bogota/jn.cordobap1/in
```

### **Formato de datos (JSON):**
```json
{
  "temperatura": 24.1,
  "humedad": 58.3, 
  "luminosidad": 450
}
```

---

## 🚨 **Sistema de Alertas**

### **Comandos soportados:**
| Comando | Mensaje ESP32 | Patrón LED | Duración |
|---------|---------------|------------|----------|
| `TEMP_HIGH` | "TEMP ALTA" | Parpadeo rápido (250ms) | 10s |
| `TEMP_LOW` | "TEMP BAJA" | Parpadeo ultra-rápido (150ms) | 10s |
| `HUMIDITY_HIGH` | "HUMEDAD ALTA" | Parpadeo lento (500ms) | 10s |
| `HUMIDITY_LOW` | "HUMEDAD BAJA" | Parpadeo rápido (200ms) | 10s |
| `LIGHT_HIGH` | "LUZ ALTA" | Parpadeo medio (350ms) | 10s |
| `LIGHT_LOW` | "LUZ BAJA" | LED fijo encendido | 10s |
| `ANOMALY` | "ANOMALIA" | Doble parpadeo (100ms) | 10s |
| `ENERGY_OPTIMIZE` | "OPTIMIZAR" | Parpadeo ultra-lento (1000ms) | 10s |
| `ENVIRONMENTAL_STRESS` | "ESTRES AMB" | Alternado rápido-lento | 10s |
| `ALERT_OFF` | "" | LED apagado | Inmediato |

### **Envío manual de alertas:**
```bash
python manage.py send_command TEMP_HIGH
```

---

## 📊 **Estructura del Proyecto**

```
IOTMonitoringServer/
├── README.md                      # Este archivo
├── manage.py                      # Django management
├── requirements.txt               # Dependencias Python
├── IOTMonitoringServer/           # Configuración Django
│   ├── settings.py
│   ├── urls.py
│   └── wsgi.py
├── control/                       # Sistema de control y alertas
│   ├── monitor.py                 # Monitor automático (3s)
│   └── management/commands/
│       └── send_command.py        # Envío manual de comandos
├── receiver/                      # Recepción y procesamiento MQTT
│   ├── models.py                  # Modelos de datos
│   ├── mqtt.py                    # Cliente MQTT
│   └── management/commands/
├── viewer/                        # Dashboard y visualización
│   ├── views.py                   # Vistas web
│   ├── templates/                 # Templates HTML
│   └── static/                    # Assets estáticos
└── hardware/                      # Firmware ESP32
    ├── iot_device_with_event_processing.ino
    └── README_CAMBIOS_IOT.md      # Documentación hardware
```

---

## 🔄 **Flujo de Datos**

```mermaid
graph LR
    A[ESP32 Sensores] -->|MQTT Publish| B[Servidor Django]
    B -->|Análisis cada 3s| C[Sistema de Alertas]
    C -->|MQTT Command| A
    A -->|Visual/LED| D[Usuario]
    B -->|Web Dashboard| D
```

### **1. Sensores → Servidor:**
```
📊 Datos: T=24.1°C, H=58.3%, Luz=450lx
Datos enviados: {"temperatura": 24.1, "humedad": 58.3, "luminosidad": 450}
```

### **2. Servidor → Análisis:**
```
Analizando datos de dispositivos...
Dispositivos encontrados: 1
Comando enviado: TEMP_HIGH
```

### **3. Comando → ESP32:**
```
Comando recibido: TEMP_HIGH
Alerta: Temperatura alta detectada
```

---

## 🌐 **Interfaz Web**

### **Rutas disponibles:**
- **`/`** - Dashboard principal con resumen
- **`/realtime/`** - Datos en tiempo real
- **`/map/`** - Mapa de dispositivos IoT
- **`/historical/`** - Análisis histórico
- **`/admin/`** - Panel de administración

### **Panel de administración:**
```bash
# Acceder con superusuario en:
http://localhost:8000/admin/
```

---

## 🎯 **Características Técnicas**

### **Hardware ESP32:**
- **Medición**: Cada 2 segundos
- **WiFi**: Reconexión automática
- **MQTT**: Reintentos cada 5 segundos
- **Sensores**: Validación de lecturas
- **Display**: Detección I2C automática (0x3C/0x3D)

### **Backend Django:**
- **Framework**: Django 4.0.3
- **Base de datos**: PostgreSQL
- **Análisis**: Cada 3 segundos
- **API**: REST para consultas
- **Dashboard**: Tiempo real con WebSockets

### **Broker MQTT:**
- **Host**: 32.192.84.37
- **Puerto**: 8082 
- **Protocolo**: MQTT v3.1.1
- **QoS**: 1 (Al menos una vez)

---

## 🛠️ **Comandos Útiles**

### **Django:**
```bash
# Ejecutar migraciones
python manage.py migrate

# Crear superusuario
python manage.py createsuperuser

# Ejecutar tests
python manage.py test

# Limpiar base de datos
python manage.py flush

# Enviar comando manual
python manage.py send_command TEMP_HIGH
```

### **Desarrollo:**
```bash
# Ver logs MQTT
python manage.py start_mqtt --verbose

# Debug modo desarrollo
python manage.py runserver --debug

# Ejecutar en modo producción
gunicorn IOTMonitoringServer.wsgi:application
```

---

## 📈 **Monitoreo y Logs**

### **Monitor Serial ESP32:**
```
=== INICIANDO SISTEMA IOT ===
✓ Pines configurados
✓ WiFi conectado: 192.168.1.100
✓ MQTT conectado
📊 Datos: T=24.1°C, H=58.3%, Luz=450lx
Comando recibido: TEMP_HIGH
Alerta: Temperatura alta detectada
```

### **Logs Django:**
```
INFO: MQTT client connected successfully
INFO: Data received from jn.cordobap1
INFO: Alert sent: TEMP_HIGH
DEBUG: 1 devices analyzed
```

---

## ⚙️ **Configuración Avanzada**

### **Variables de entorno (.env):**
```env
DEBUG=True
SECRET_KEY=tu_secret_key
DATABASE_URL=postgresql://user:pass@localhost/iot_db
MQTT_HOST=32.192.84.37
MQTT_PORT=8082
```

### **Base de datos (settings.py):**
```python
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.postgresql',
        'HOST': '54.167.96.107',
        'PORT': '5432',
        'NAME': 'iot_data',
        'USER': 'dbadmin',
        'PASSWORD': 'uniandesIOT1234*'
    }
}
```

### **Configuración MQTT:**
```python
MQTT_HOST = "32.192.84.37"
MQTT_PORT = 8082
MQTT_USER = "jn.cordobap1"
MQTT_PASS = "abc123"
```

---

## 🎯 **Estado del Proyecto**

**✅ Funcionalidades Operativas:**
- Sistema completo ESP32 + Django funcional
- Comunicación MQTT bidireccional estable
- Sistema de alertas con 9 tipos de comandos
- Dashboard web en tiempo real
- Base de datos PostgreSQL integrada
- Manejo robusto de errores y reconexiones

**🔄 En desarrollo:**
- Optimizaciones de rendimiento
- Alertas por email/SMS
- Análisis predictivo con ML
- Aplicación móvil

---

## 🔧 **Troubleshooting**

### **🚨 Problemas comunes:**

#### **ESP32 no conecta a WiFi:**
```cpp
// Verificar credenciales en ino:
const char ssid[] = "TU_WIFI_EXACTO";
const char pass[] = "TU_PASSWORD_EXACTO";
```

#### **"0 devices reviewed" en servidor:**
```bash
# 1. Verificar que start_mqtt esté corriendo
python manage.py start_mqtt

# 2. Verificar broker MQTT
telnet 32.192.84.37 8082

# 3. Revisar tópicos MQTT
mosquitto_sub -h 32.192.84.37 -p 8082 -t "colombia/cundinamarca/bogota/+/out"
```

#### **Base de datos no conecta:**
```bash
# Verificar conexión PostgreSQL
psql -h 54.167.96.107 -p 5432 -U dbadmin -d iot_data
```

---

**📅 Última actualización**: Marzo 3, 2026  
**🏗️ Versión**: 1.0 - Sistema completo funcional  
**✅ Estado**: Listo para producción

---

## 📖 **Documentación Adicional**

- **Hardware detallado**: `hardware/README_CAMBIOS_IOT.md`
- **API REST**: Documentación en `/admin/doc/`
- **Logs detallados**: Archivos de log de Django
- **Monitoreo ESP32**: Monitor Serial Arduino IDE

## 👨‍💻 **Soporte Técnico**

Para soporte o contribuciones:
- **Tests**: `python manage.py test`
- **Debug MQTT**: Verificar broker en puerto 8082
- **Logs**: Revisar Monitor Serial ESP32 y logs Django
- **Issues**: Documentar problemas con logs completos

### Envío desde dispositivo (`/out`):
```json
{
    "temperatura": 25.5,
    "humedad": 60.2,
    "luminosidad": 350
}
```

### Comandos hacia dispositivo (`/in`):
```
TEMP_HIGH
HUMIDITY_HIGH
LIGHT_LOW
ANOMALY
ENERGY_OPTIMIZE
ENVIRONMENTAL_STRESS
ALERT_OFF
```

## 🔒 Seguridad

- Cambiar `SECRET_KEY` en producción
- Configurar `DEBUG = False` en producción
- Usar variables de entorno para credenciales
- Habilitar TLS para MQTT si es necesario

## 🐛 Troubleshooting

### Error de conexión MQTT:
- Verificar credenciales en `settings.py`
- Comprobar conectividad al broker

### Error de base de datos:
- Verificar configuración PostgreSQL
- Ejecutar migraciones

### Dispositivo no aparece:
- Verificar configuración WiFi en .ino
- Comprobar credenciales MQTT
- Revisar tópicos MQTT

## 📝 Logs

El sistema genera logs detallados en:
- Console del servicio MQTT
- Console del servicio de control
- Monitor Serial del ESP32