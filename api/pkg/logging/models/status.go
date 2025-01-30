package models

type HouseStatus struct {
	HouseId string `json:"house_id"`

	RoomStatuses []RoomStatus
}

// RoomStatus is the status of the room's environment,
// as well as the PeripheralStatus(s) of the peripherals it manages
type RoomStatus struct {
	RoomID string `json:"room_id"`
	Temp   string `json:"temp"`
	Hum    string `json:"hum"`
	Co2    string `json:"co2"`
	Tvoc   string `json:"tvoc"`
	Nox    string `json:"nox"`
	Pm1    string `json:"pm1"`
	Pm25   string `json:"pm25"`
	Pm10   string `json:"pm10"`

	SensorStatuses     []SensorStatus     `json:"sensor_statuses"`
	PeripheralStatuses []PeripheralStatus `json:"peripheral_statuses"`
}

// SelfStatus is the status of the monitor's internal components,
// as well as the SelfStatus(s) of the peripherals it manages
type SensorStatus struct {
	RoomID  string `json:"room_id"`
	Battery string `json:"battery"`
	Uptime  string `json:"uptime"`
	Model   string `json:"model"`
}

// PeripheralStatus is the status of a peripheral
// Peripherals are the devices that share more info about the room's environment, dynamically.
// Peripherals are optional
// e.g. door sensors, people counters, etc.
type PeripheralStatus struct {
	RoomID         string `json:"room_id"`
	PeripheralID   string `json:"peripheral_id"`
	Battery        string `json:"battery"`
	Uptime         string `json:"uptime"`
	Model          string `json:"model"`
	MonitoringWhat string `json:"monitoring_what"` // "people count", "door opened or closed", "window opened or closed", etc.
}
