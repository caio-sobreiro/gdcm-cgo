#include <gdcmReader.h>
#include <gdcmFileDerivation.h>
#include <gdcmAnonymizer.h>
#include <gdcmVersion.h>
#include <gdcmPixmapReader.h>
#include <gdcmPixmapWriter.h>
#include <gdcmWriter.h>
#include <gdcmSystem.h>
#include <gdcmFileMetaInformation.h>
#include <gdcmDataSet.h>
#include <gdcmIconImageGenerator.h>
#include <gdcmAttribute.h>
#include <gdcmSequenceOfItems.h>
#include <gdcmUIDGenerator.h>
#include <gdcmImage.h>
#include <gdcmImageChangeTransferSyntax.h>
#include <gdcmImageApplyLookupTable.h>
#include <gdcmFileDecompressLookupTable.h>
#include <gdcmImageFragmentSplitter.h>
#include <gdcmImageChangePlanarConfiguration.h>
#include <gdcmImageChangePhotometricInterpretation.h>
#include <gdcmFileExplicitFilter.h>
#include <gdcmJPEG2000Codec.h>
#include <gdcmJPEGCodec.h>
#include <gdcmJPEGLSCodec.h>
#include <gdcmSequenceOfFragments.h>

#include <string>
#include <iostream>

#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <getopt.h>
#include <string.h>

int change_transfersyntax(const std::string &filename, const std::string &outfilename)
{
  gdcm::Reader reader;
  reader.SetFileName(filename.c_str());
  if (!reader.Read())
  {
    std::cerr << "Could not read: " << filename << std::endl;
    return 1;
  }
  gdcm::MediaStorage ms;
  ms.SetFromFile(reader.GetFile());
  if (ms == gdcm::MediaStorage::MediaStorageDirectoryStorage)
  {
    std::cerr << "Sorry DICOMDIR is not supported" << std::endl;
    return 1;
  }

  gdcm::Writer writer;
  writer.SetFileName(outfilename.c_str());
  writer.SetFile(reader.GetFile());
  gdcm::File &file = writer.GetFile();
  gdcm::FileMetaInformation &fmi = file.GetHeader();

  const gdcm::TransferSyntax &orits = fmi.GetDataSetTransferSyntax();
  if (orits != gdcm::TransferSyntax::ExplicitVRLittleEndian && orits != gdcm::TransferSyntax::ImplicitVRLittleEndian && orits != gdcm::TransferSyntax::DeflatedExplicitVRLittleEndian)
  {
    std::cerr << "Sorry input Transfer Syntax not supported for this conversion: " << orits << std::endl;
    return 1;
  }

  gdcm::TransferSyntax ts = gdcm::TransferSyntax::ImplicitVRLittleEndian;
  std::string tsuid = gdcm::TransferSyntax::GetTSString(ts);
  if (tsuid.size() % 2 == 1)
  {
    tsuid.push_back(0); // 0 padding
  }
  gdcm::DataElement de(gdcm::Tag(0x0002, 0x0010));
  de.SetByteValue(tsuid.data(), (uint32_t)tsuid.size());
  de.SetVR(gdcm::Attribute<0x0002, 0x0010>::GetVR());
  fmi.Clear();
  fmi.Replace(de);

  fmi.SetDataSetTransferSyntax(ts);

  if (!writer.Write())
  {
    std::cerr << "Failed to write: " << outfilename << std::endl;
    return 1;
  }

  return 0;
}

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

  gdcm::JPEG2000Codec j2kcodec;
  gdcm::JPEGCodec jpegcodec;
  gdcm::JPEGLSCodec jpeglscodec;
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
