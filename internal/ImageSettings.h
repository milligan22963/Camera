/**
 * ImageSettings.h
 */

 #ifndef _H_IMAGE_SETTINGS
 #define _H_IMAGE_SETTINGS

#include <cstdint>
#include <memory>

#include "Table.h"
#include "DataTypes.h"

class ImageSettings : public Table
{
    public:
        ImageSettings();
        virtual ~ImageSettings();

        std::string GetNextImageName();

        afm::data::ImageType GetImageType() { return m_image_type; }

        void SetFilename(const std::string &filename) { m_image_filename = filename; }
        void SetLocation(const std::string &location) { m_image_location = location; }
        void SetNextImageNumber(uint32_t next_image_number) { m_next_image_number = next_image_number; }
        void SetImageType(afm::data::ImageType image_type) { m_image_type = image_type; }

    protected:
        virtual std::string OnGetCreateString() override;
        virtual void OnSaveData() override;
        void LoadCurrentData();

    private:
        bool m_data_loaded = false;
        afm::data::ImageType m_image_type = afm::data::ImageType::IMG_JPEG;
        std::string m_image_location = "/home/pi/Documents";
        std::string m_image_filename = "image_";
        uint32_t m_next_image_number = 1;
};

using ImageSettingsSPtr = std::shared_ptr<ImageSettings>;

 #endif