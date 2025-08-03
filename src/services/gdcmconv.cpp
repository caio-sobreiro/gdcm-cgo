#include <gdcmReader.h>
#include <gdcmPixmapReader.h>
#include <gdcmPixmapWriter.h>
#include <gdcmWriter.h>
#include <gdcmAttribute.h>
#include <gdcmImageChangeTransferSyntax.h>

extern "C" int convert_to_jpeg2000(const std::string &filename, const std::string &outfilename)
{
  gdcm::FileMetaInformation::SetSourceApplicationEntityTitle("gdcmconv");

  gdcm::PixmapReader reader;
  reader.SetFileName(filename.c_str());
  if (!reader.Read())
  {
    gdcm::MediaStorage ms;
    ms.SetFromFile(reader.GetFile());
    std::cerr << "Could not read (pixmap): " << filename << std::endl;
    return 1;
  }
  gdcm::Pixmap &image = reader.GetPixmap();

  gdcm::ImageChangeTransferSyntax change;
  change.SetForce(false);
  change.SetCompressIconImage(false);
  change.SetTransferSyntax(gdcm::TransferSyntax::JPEG2000Lossless);

  change.SetInput(image);

  bool b = change.Change();
  if (!b)
  {
    std::cerr << "Could not change the Transfer Syntax: " << filename << std::endl;
    return 1;
  }
  gdcm::PixmapWriter writer;
  writer.SetFileName(outfilename.c_str());
  writer.SetFile(reader.GetFile());

  gdcm::File &file = writer.GetFile();
  gdcm::FileMetaInformation &fmi = file.GetHeader();
  fmi.Remove(gdcm::Tag(0x0002, 0x0100));
  fmi.Remove(gdcm::Tag(0x0002, 0x0102));

  const gdcm::Pixmap &pixout = change.PixmapToPixmapFilter::GetOutput();
  writer.SetPixmap(pixout);
  if (!writer.Write())
  {
    std::cerr << "Failed to write: " << outfilename << std::endl;
    return 1;
  }

  return 0;
}
