/**
 * ImageSettings.cpp
 */

#include <iomanip>
#include <sstream>
#include "ImageSettings.h"

const std::string sc_image_name = "name";
const std::string sc_image_location = "location";
const std::string sc_image_number = "number";
const std::string sc_image_type = "type";
const std::string sc_image_settings_table_name = "image_settings";
const std::string sc_image_settings_table_setting = "setting";
const std::string sc_image_settings_table_value = "value";

int image_settings_callback(void *p_table, int col_count, char **pp_data, char **pp_columns);

ImageSettings::ImageSettings()
    : Table()
{
    SetTableName(sc_image_settings_table_name);
}

ImageSettings::~ImageSettings()
{

}

std::string ImageSettings::GetNextImageName()
{
    std::stringstream next_image_name;

    if (m_data_loaded == false)
    {
        LoadCurrentData();
    }

    next_image_name << m_image_location << "/";
    next_image_name << m_image_filename;
    next_image_name << std::right << std::setfill('0') << std::setw(8) << (int)m_next_image_number++;
    next_image_name << "." << afm::data::sc_image_extensions[m_image_type];

    return next_image_name.str();
}


// internal
std::string ImageSettings::OnGetCreateString()
{
    std::stringstream create_string;

    create_string << sc_image_settings_table_setting << " TEXT NOT NULL,";
    create_string << sc_image_settings_table_value << " TEXT NOT NULL";

    return create_string.str();
}

void ImageSettings::LoadCurrentData()
{
    std::stringstream load_command;

    // create load command
    load_command << "select * from ";
    load_command << sc_image_settings_table_name << ";";

    if (GetDatabase()->IssueCommand(load_command.str(), image_settings_callback, this) == true)
    {

    }
    m_data_loaded = true;
}

void ImageSettings::OnSaveData()
{
    std::stringstream save_command;

    // INSERT OR REPLACE INTO prod_mast(prod_id, prod_name, prod_rate, prod_qc)
    // VALUES(4, 'Pizza', 200, 'OK');
    save_command << "INSERT OR REPLACE INTO " << sc_image_settings_table_name << "(";
    save_command << sc_image_settings_table_setting << ", " << sc_image_settings_table_value << ") ";
    save_command << "VALUES (";

    GetDatabase()->IssueCommand(save_command.str() + sc_image_location + "," + m_image_location + ")");
    GetDatabase()->IssueCommand(save_command.str() + sc_image_type + "," + afm::data::sc_image_extensions[m_image_type] + ")");
    GetDatabase()->IssueCommand(save_command.str() + sc_image_name + "," + m_image_filename + ")");
    GetDatabase()->IssueCommand(save_command.str() + sc_image_number + "," + std::to_string(m_next_image_number) + ")");
}

int image_settings_callback(void *p_table, int col_count, char **pp_data, char **pp_columns)
{
    ImageSettings *p_image_settings = static_cast<ImageSettings *>(p_table);

    if (col_count == 2)
    {
        if (sc_image_name == pp_data[0])
        {
            p_image_settings->SetFilename(pp_data[1]);
        }
        else if (sc_image_location == pp_data[0])
        {
            p_image_settings->SetLocation(pp_data[1]);
        }
        else if (sc_image_number == pp_data[0])
        {
            uint32_t next_image_number = std::stoul(pp_data[1]);
            p_image_settings->SetNextImageNumber(next_image_number);
        }
        else if (sc_image_type == pp_data[0])
        {
            for (int index = 0; index < afm::data::ImageType::END_IMAGE_TYPES; index++)
            {
                if (afm::data::sc_image_extensions[index] == pp_data[1])
                {
                    p_image_settings->SetImageType((afm::data::ImageType)index);
                }
            }
        }
    }
    return 0;
}
