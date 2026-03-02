from django.core.management.base import BaseCommand
from django.conf import settings
import paho.mqtt.client as mqtt
import ssl


class Command(BaseCommand):
    help = 'Sends manual commands to IoT devices'

    def add_arguments(self, parser):
        parser.add_argument('--user', type=str, required=True,
                            help='Username of the device')
        parser.add_argument('--city', type=str, required=True,
                            help='City name')
        parser.add_argument('--state', type=str, required=True,
                            help='State name')
        parser.add_argument('--country', type=str, required=True,
                            help='Country name')
        parser.add_argument('--command', type=str, required=True,
                            choices=['TEMP_HIGH', 'TEMP_LOW', 'HUMIDITY_HIGH', 'HUMIDITY_LOW',
                                   'LIGHT_LOW', 'LIGHT_HIGH', 'ANOMALY', 'ENERGY_OPTIMIZE', 
                                   'ENVIRONMENTAL_STRESS', 'ALERT_OFF'],
                            help='Command to send to device')

    def handle(self, *args, **options):
        # Configurar cliente MQTT
        client = mqtt.Client(settings.MQTT_USER_PUB)
        
        if settings.MQTT_USE_TLS:
            client.tls_set(ca_certs=settings.CA_CRT_PATH,
                           tls_version=ssl.PROTOCOL_TLSv1_2, 
                           cert_reqs=ssl.CERT_NONE)

        client.username_pw_set(settings.MQTT_USER_PUB, 
                               settings.MQTT_PASSWORD_PUB)
        
        # Conectar
        client.connect(settings.MQTT_HOST, settings.MQTT_PORT, 60)
        
        # Construir tópico
        topic = f"{options['country']}/{options['state']}/{options['city']}/{options['user']}/in"
        
        # Enviar comando
        result = client.publish(topic, options['command'])
        
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            self.stdout.write(
                self.style.SUCCESS(f'Command "{options["command"]}" sent successfully to {topic}')
            )
        else:
            self.stdout.write(
                self.style.ERROR(f'Failed to send command. Error code: {result.rc}')
            )
        
        client.disconnect()