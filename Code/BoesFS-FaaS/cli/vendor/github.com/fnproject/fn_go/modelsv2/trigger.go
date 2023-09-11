// Code generated by go-swagger; DO NOT EDIT.

package modelsv2

// This file was generated by the swagger tool.
// Editing this file might prove futile when you re-run the swagger generate command

import (
	"github.com/go-openapi/errors"
	"github.com/go-openapi/strfmt"
	"github.com/go-openapi/swag"
	"github.com/go-openapi/validate"
)

// Trigger trigger
//
// swagger:model Trigger
type Trigger struct {

	// Trigger annotations - this is a map of annotations attached to this trigger, keys must not exceed 128 bytes and must consist of non-whitespace printable ascii characters, and the seralized representation of individual values must not exeed 512 bytes.
	Annotations map[string]interface{} `json:"annotations,omitempty"`

	// Opaque, unique Application identifier
	// Read Only: true
	AppID string `json:"app_id,omitempty"`

	// Time when trigger was created. Always in UTC.
	// Read Only: true
	// Format: date-time
	CreatedAt strfmt.DateTime `json:"created_at,omitempty"`

	// Opaque, unique Function identifier
	// Read Only: true
	FnID string `json:"fn_id,omitempty"`

	// Unique Trigger identifier.
	// Read Only: true
	ID string `json:"id,omitempty"`

	// Unique name for this trigger, used to identify this trigger.
	Name string `json:"name,omitempty"`

	// URI path for this trigger. e.g. `sayHello`, `say/hello`
	Source string `json:"source,omitempty"`

	// Class of trigger, e.g. schedule, http, queue
	Type string `json:"type,omitempty"`

	// Most recent time that trigger was updated. Always in UTC.
	// Read Only: true
	// Format: date-time
	UpdatedAt strfmt.DateTime `json:"updated_at,omitempty"`
}

// Validate validates this trigger
func (m *Trigger) Validate(formats strfmt.Registry) error {
	var res []error

	if err := m.validateCreatedAt(formats); err != nil {
		res = append(res, err)
	}

	if err := m.validateUpdatedAt(formats); err != nil {
		res = append(res, err)
	}

	if len(res) > 0 {
		return errors.CompositeValidationError(res...)
	}
	return nil
}

func (m *Trigger) validateCreatedAt(formats strfmt.Registry) error {

	if swag.IsZero(m.CreatedAt) { // not required
		return nil
	}

	if err := validate.FormatOf("created_at", "body", "date-time", m.CreatedAt.String(), formats); err != nil {
		return err
	}

	return nil
}

func (m *Trigger) validateUpdatedAt(formats strfmt.Registry) error {

	if swag.IsZero(m.UpdatedAt) { // not required
		return nil
	}

	if err := validate.FormatOf("updated_at", "body", "date-time", m.UpdatedAt.String(), formats); err != nil {
		return err
	}

	return nil
}

// MarshalBinary interface implementation
func (m *Trigger) MarshalBinary() ([]byte, error) {
	if m == nil {
		return nil, nil
	}
	return swag.WriteJSON(m)
}

// UnmarshalBinary interface implementation
func (m *Trigger) UnmarshalBinary(b []byte) error {
	var res Trigger
	if err := swag.ReadJSON(b, &res); err != nil {
		return err
	}
	*m = res
	return nil
}