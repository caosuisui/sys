//
// Created by wyz on 2022/4/28.
//
#include "VolumeProvider.hpp"

#include <cassert>
#include <cstdint>
//struct Decoder{
//    static size_t decode(AVCodecContext* c,AVFrame* frame,AVPacket* pkt,uint8_t* buf){
//        int ret = avcodec_send_packet(c,pkt);
//        if(ret < 0){
//            throw std::runtime_error("error sending a packet for decoding");
//        }
//        size_t frame_pos = 0;
//        while(ret >= 0){
//            ret = avcodec_receive_frame(c,frame);
//            if(ret == AVERROR(EAGAIN) || ret ==AVERROR_EOF)
//                break;
//            else if(ret < 0){
//                throw std::runtime_error("error during decoding");
//            }
//            memcpy(buf + frame_pos,frame->data[0],frame->linesize[0]*frame->height);
//            frame_pos += frame->linesize[0] * frame->height;
//        }
//        return frame_pos;
//    }
//
//    static void uncompress(void* data,size_t len,std::vector<std::vector<uint8_t>>& packets){
//        auto codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
//        assert(codec);
////        auto parser = av_parser_init(codec->id);
////        assert(parser);
//
//        auto c = avcodec_alloc_context3(codec);
//        c->thread_count = 6;
//        c->delay = 0;
//        assert(c);
//        int ret = avcodec_open2(c,codec,nullptr);
//        assert(ret >= 0);
//        auto frame = av_frame_alloc();
//        assert(frame);
//        auto pkt = av_packet_alloc();
//        assert(pkt);
//        uint8_t* p = (uint8_t*)data;
//        size_t offset = 0;
//
//        for(auto& packet:packets){
//            pkt->data = packet.data();
//            pkt->size = packet.size();
//            offset += decode(c,frame,pkt,p+offset);
//            if(offset > len){
//                throw std::runtime_error("decode result out of buffer range");
//            }
//        }
//        decode(c,frame,nullptr,p+offset);
//
//        avcodec_free_context(&c);
//        av_frame_free(&frame);
//        av_packet_free(&pkt);
//    }
//};
VolumeProvider::VolumeProvider(const std::string &filename) {
    reader = std::make_unique<Reader>(filename.c_str());
    reader->read_header();
    VoxelUncompressOptions opt;
    opt.codec_method = cudaVideoCodec_HEVC;
    uncmp = std::make_unique<VoxelUncompress>(opt);
}

bool VolumeProvider::readBlock(const VolumeProvider::BlockIndex &index, void *buf,size_t size) {
    std::vector<std::vector<uint8_t>> packets;
    reader->read_packet({(uint32_t)index.x,(uint32_t)index.y,(uint32_t)index.z},packets);
    if(packets.empty()) return false;
//    Decoder::uncompress(buf,size,packets);
    uncmp->uncompress(reinterpret_cast<uint8_t*>(buf),size,packets);
    std::cerr<<"readBlock: "<<index.x<<" "<<index.y<<" "<<index.z<<std::endl;
    return true;
}

VolumeProvider::VolumeInfo VolumeProvider::getVolumeInfo() {
    VolumeInfo info{};
    auto h = reader->get_header();
    info.block_length = 1 << h.log_block_length;
    info.padding = h.padding;
    info.raw_x = h.raw_x;
    info.raw_y = h.raw_y;
    info.raw_z = h.raw_z;
    return info;
}
