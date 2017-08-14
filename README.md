# Async_Monitor_UDP
The same project as AsyncMonitor_UDP. But on another comuter.
The application reads all the necessary information from the config.json.
The configuration file must contain 4 required fields:
	"captureRecordFile" is the path to the file in which the result of the monitor's operation will be recorded (string);
	"urlResource" is URL of the site from which the list of servers for monitoring will be received (string);
	"period_sending_request_ms" is the period of sending request in milliseconds (integer);
	"max_number_request_sent" is the maximum number of request sent (integer). The server is considered non-working if it does not provide a response for the specified number of request;